/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_entry_bits.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "gtest/gtest.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::Return;

struct LegacyPageTableTest : public MockAubStreamFixture, public ::testing::Test {
    uint32_t memoryBank = defaultMemoryBank;

    void SetUp() override {
        MockAubStreamFixture::SetUp();
    }

    void TearDown() override {
        MockAubStreamFixture::TearDown();
    }
};

struct LegacyPageTableParamTest : public ::testing::TestWithParam<uint32_t> {
    uint32_t memoryBank = defaultMemoryBank;

    void SetUp() override {
        memoryBank = GetParam();
    }
};

using PML4LegacyTest = LegacyPageTableParamTest;
using PDELegacyTest = LegacyPageTableParamTest;
using PDPLegacyTest = LegacyPageTableParamTest;
using PDP4LegacyTest = LegacyPageTableParamTest;
using PTE64KBLegacyTest = LegacyPageTableParamTest;

static uint32_t nonColoredLocalMemoryBanks[] = {
    MEMORY_BANK_0,
};

static uint32_t allMemoryBanks[] = {
    MEMORY_BANK_SYSTEM,
    MEMORY_BANK_0,
};

TEST_P(PML4LegacyTest, ctor) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(LegacyPML4,
                         PML4LegacyTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(LegacyPML4, getEntryValue) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(LegacyPML4, getIndex) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(130u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(270u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST(LegacyPDP4, getEntryValue) {
    PhysicalAddressAllocator allocator;
    LegacyPDP4 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(LegacyPDP4, getIndex) {
    PhysicalAddressAllocator allocator;
    LegacyPDP4 pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0u));
    EXPECT_EQ(1u, pageTable.getIndex(0x41234567));
    EXPECT_EQ(2u, pageTable.getIndex(0x87654321));
    EXPECT_EQ(3u, pageTable.getIndex(0xffffffff));
}

TEST(LegacyPDP4, allocateChildReturnsNonNullPointer) {
    PhysicalAddressAllocator allocator;
    LegacyPDP4 pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto child = pageTable.allocateChild(*gpu, 4096, pageTable.getMemoryBank());
    EXPECT_NE(nullptr, child);
    delete child;
}

TEST_P(PDP4LegacyTest, ctor) {
    PhysicalAddressAllocator allocator;
    LegacyPDP4 pageTable(*gpu, &allocator, memoryBank);
    EXPECT_EQ(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(LegacyPDP4,
                         PDP4LegacyTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(LegacyPDP, getEntryValue) {
    PhysicalAddressAllocator allocator;
    LegacyPDP pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST_P(PDPLegacyTest, ctor) {
    PhysicalAddressAllocator allocator;
    LegacyPDP pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(LegacyPDP,
                         PDPLegacyTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(LegacyPDE, getEntryValue) {
    PhysicalAddressAllocator allocator;
    LegacyPDE pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(LegacyPDE, getIndex) {
    PhysicalAddressAllocator allocator;
    LegacyPDE pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(43u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(25u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PDELegacyTest, ctor) {
    PhysicalAddressAllocator allocator;
    LegacyPDE pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(LegacyPDE,
                         PDELegacyTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(LegacyPTE64KB, getEntryValue) {
    PhysicalAddressAllocator allocator;
    LegacyPTE64KB pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::legacyIntermediatePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(LegacyPTE64KB, getIndex) {
    PhysicalAddressAllocator allocator;
    LegacyPTE64KB pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0x00, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(0x70, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(0x10, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(0x1f0, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PTE64KBLegacyTest, ctor) {
    PhysicalAddressAllocator allocator;
    LegacyPTE64KB pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(LegacyPTE64KB,
                         PTE64KBLegacyTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST_F(LegacyPageTableTest, GivenHBMTableAndSystemLastLevelPageVerifyMemoryLocation) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, defaultPageSize});

    EXPECT_TRUE(ppgtt.isLocalMemory());

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto pte = pde->getChild(0);
    ASSERT_NE(nullptr, pte);
    EXPECT_TRUE(pte->isLocalMemory());

    auto page = pte->getChild(0);
    ASSERT_NE(nullptr, page);
    EXPECT_FALSE(page->isLocalMemory());
}

TEST_F(LegacyPageTableTest, Write4KBFollowedBy64KBPageSizeShouldNotChange) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    size_t size1 = 0x2000;
    size_t size2 = 0xc0;
    auto data1 = new uint8_t[size1];
    auto data2 = new uint8_t[size2];
    memset(data1, 0x1, size1);
    memset(data2, 0x2, size2);

    uint64_t address = 0x0;
    size_t pageSize1 = 0x1000;
    size_t pageSize2 = 0x10000;

    stream.writeMemory(&ppgtt, {address, data1, size1, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize1});
    stream.writeMemory(&ppgtt, {address, data2, size2, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize2});

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto pte = static_cast<PTE *>(pde->getChild(0));
    ASSERT_NE(nullptr, pte);
    EXPECT_EQ(pageSize1, pte->getPageSize());
}

TEST_F(LegacyPageTableTest, Write64KBFollowedBy4KBPageSizeShouldNotChange) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    size_t size1 = 0x2000;
    size_t size2 = 0xc0;
    auto data1 = new uint8_t[size1];
    auto data2 = new uint8_t[size2];
    memset(data1, 0x1, size1);
    memset(data2, 0x2, size2);

    uint64_t address = 0x0;
    size_t pageSize1 = 0x10000;
    size_t pageSize2 = 0x1000;

    stream.writeMemory(&ppgtt, {address, data1, size1, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize1});
    stream.writeMemory(&ppgtt, {address, data2, size2, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize2});

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto pte = static_cast<PTE *>(pde->getChild(0));
    ASSERT_NE(nullptr, pte);
    EXPECT_EQ(pageSize1, pte->getPageSize());
}

TEST_F(LegacyPageTableTest, GivenHBMPML4LegacyTableAndLastLevelPageSystemPageVerifyNewPageTableHintsAreUsed) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});
}

TEST_F(LegacyPageTableTest, GivenHBMPML4LegacyTableAndSystemLastLevel64KBPageVerifyReserveContiguousPagesIsCalled) {
    PhysicalAddressAllocator allocator;
    LegacyPML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, reserveContiguousPages(_)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});
}

TEST_F(LegacyPageTableTest, GivenHBMGGTTAubStreamWriteMemoryVerifyChildPageAttributes) {
    PhysicalAddressAllocator allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ggtt, 0, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 4096);

    auto child = ggtt.getChild(0);
    ASSERT_NE(nullptr, child);
    EXPECT_NE(0u, child->getPhysicalAddress());
    EXPECT_EQ(ggtt.getMemoryBank(), child->getMemoryBank());
    EXPECT_TRUE(child->isLocalMemory());
}

TEST_F(LegacyPageTableTest, GivenSystemmemGGTTAubStreamWriteMemoryVerifyChildPageAttributes) {
    PhysicalAddressAllocator allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ggtt, 0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 4096);

    auto child = ggtt.getChild(0);
    ASSERT_NE(nullptr, child);
    EXPECT_NE(0u, child->getPhysicalAddress());
    EXPECT_EQ(ggtt.getMemoryBank(), child->getMemoryBank());
    EXPECT_FALSE(child->isLocalMemory());
}

TEST_F(LegacyPageTableTest, GivenLocalMemoryGGTTWhenCallingExpectMemoryThenExpectMemoryTableGetsCalled) {
    PhysicalAddressAllocator allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, expectMemoryTable(_, _, _, _)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ggtt, 0, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 4096);
    stream.expectMemory(&ggtt, 0, &data, sizeof(data), 0);
}
