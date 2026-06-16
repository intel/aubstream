/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/physical_address_allocator.h"
#include "aubstream/allocation_params.h"
#include "aubstream/hint_values.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "test.h"
#include "gtest/gtest.h"

using namespace aub_stream;

namespace {

constexpr size_t pageSize4K = 4096;

struct ExpectWalkReleaseTest : public MockAubStreamFixture, public ::testing::Test {
    void SetUp() override { MockAubStreamFixture::SetUp(); }
    void TearDown() override { MockAubStreamFixture::TearDown(); }
};

} // namespace

TEST_F(ExpectWalkReleaseTest, givenUnmappedGgttAddressWhenReadMemoryThenNoGttNodeCreated) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    const uint64_t gfxAddress = 0x1000;
    const auto index = ggtt.getIndex(static_cast<uint32_t>(gfxAddress));
    ASSERT_EQ(nullptr, ggtt.getChild(index));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ggtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, pageSize4K);

    EXPECT_EQ(nullptr, ggtt.getChild(index));
}

TEST_F(ExpectWalkReleaseTest, givenCompletelyUnmappedPpgttWhenReadMemoryThenNoNodesCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    const uint64_t gfxAddress = 0x200000;
    ASSERT_EQ(nullptr, ppgtt.getChild(ppgtt.getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, 65536);

    EXPECT_EQ(nullptr, ppgtt.getChild(ppgtt.getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenMissingPdpChildWhenReadMemoryThenNoPdeNodeCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt, {0x200000, nullptr, 65536, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});

    const uint64_t gfxAddress = 0x40000000;
    auto *pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    ASSERT_EQ(nullptr, pdp->getChild(pdp->getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, 65536);

    EXPECT_EQ(nullptr, pdp->getChild(pdp->getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenMissingPteNodeWhenReadMemoryThenNoPte64KbNodeCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt, {0x200000, nullptr, 65536, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});

    const uint64_t gfxAddress = 0x400000;
    auto *pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto *pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    ASSERT_EQ(nullptr, pde->getChild(pde->getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, 65536);

    EXPECT_EQ(nullptr, pde->getChild(pde->getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenMissingDataPageWhenReadMemoryThenNoDataPageCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt, {0x200000, nullptr, 65536, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});

    const uint64_t gfxAddress = 0x280000;
    auto *pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto *pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    auto *pte = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pte);
    ASSERT_EQ(nullptr, pte->getChild(pte->getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, 65536);

    EXPECT_EQ(nullptr, pte->getChild(pte->getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenPartiallyMappedRangeWhenReadMemoryThenUnmappedSlotRemainsNull) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt, {0x100000, nullptr, pageSize4K, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize4K});

    auto *pdp = ppgtt.getChild(ppgtt.getIndex(0x101000));
    ASSERT_NE(nullptr, pdp);
    auto *pde = pdp->getChild(pdp->getIndex(0x101000));
    ASSERT_NE(nullptr, pde);
    auto *pte = pde->getChild(pde->getIndex(0x101000));
    ASSERT_NE(nullptr, pte);
    ASSERT_EQ(nullptr, pte->getChild(pte->getIndex(0x101000)));

    uint8_t buf[2 * pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, 0x100000, buf, sizeof(buf), MEMORY_BANK_SYSTEM, pageSize4K);

    EXPECT_EQ(nullptr, pte->getChild(pte->getIndex(0x101000)));
}

TEST_F(ExpectWalkReleaseTest, givenUnmappedPpgttWith4KbPagesWhenReadMemoryThenNoNodesCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    const uint64_t gfxAddress = 0x5000;
    ASSERT_EQ(nullptr, ppgtt.getChild(ppgtt.getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, pageSize4K);

    EXPECT_EQ(nullptr, ppgtt.getChild(ppgtt.getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenMissing4KbDataPageWhenReadMemoryThenNoDataPageCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt, {0x5000, nullptr, pageSize4K, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize4K});

    const uint64_t gfxAddress = 0x6000;
    auto *pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto *pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    auto *pte = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pte);
    ASSERT_EQ(nullptr, pte->getChild(pte->getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, pageSize4K);

    EXPECT_EQ(nullptr, pte->getChild(pte->getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenMissing2MbPageWhenReadMemoryThenNoPageCreated) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator(2, aub_stream::GB, true);
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    stream.writeMemory(&ppgtt, {0x200000, nullptr, Page2MB::pageSize2MB, MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    const uint64_t gfxAddress = 0x400000;
    auto *pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto *pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    ASSERT_EQ(nullptr, pde->getChild(pde->getIndex(gfxAddress)));

    uint8_t buf[pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_0, Page2MB::pageSize2MB);

    EXPECT_EQ(nullptr, pde->getChild(pde->getIndex(gfxAddress)));
}

TEST_F(ExpectWalkReleaseTest, givenMultiPageReadOverMissingIntermediateNodeWhenReadMemoryThenNoNodesCreated) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt, {0x200000, nullptr, pageSize4K, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize4K});

    const uint64_t gfxAddress = 0x40000000;
    auto *pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    ASSERT_EQ(nullptr, pdp->getChild(pdp->getIndex(gfxAddress)));

    uint8_t buf[2 * pageSize4K] = {};
    stream.AubStream::readMemory(&ppgtt, gfxAddress, buf, sizeof(buf), MEMORY_BANK_SYSTEM, pageSize4K);

    EXPECT_EQ(nullptr, pdp->getChild(pdp->getIndex(gfxAddress)));
}
