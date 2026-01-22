/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_walker.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "aub_mem_dump/xe3p_core/command_streamer_helper_xe3p_core.h"
#include "aubstream/product_family.h"
#include "test_defaults.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST(IsMemorySupportedCri, givenLocalMemoryWhen2MBAlignmentThenIsSupported) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);
    TEST_REQUIRES(localMemorySupportedInTests);

    auto &csHelper = gpu->getCommandStreamerHelper(0, EngineType::ENGINE_CCS);

    EXPECT_TRUE(csHelper.isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));
    EXPECT_TRUE(csHelper.isMemorySupported(MEMORY_BANK_1, Page2MB::pageSize2MB));
}

TEST(IsMemorySupportedCri, givenSystemMemoryWhen2MBAlignmentThenIsNotSupported) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);

    auto &csHelper = gpu->getCommandStreamerHelper(0, EngineType::ENGINE_CCS);

    EXPECT_FALSE(csHelper.isMemorySupported(MEMORY_BANK_SYSTEM, Page2MB::pageSize2MB));
}

TEST(IsMemorySupportedCri, givenLocalMemoryWhen64KBAlignmentThenIsSupported) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);
    TEST_REQUIRES(localMemorySupportedInTests);

    auto &csHelper = gpu->getCommandStreamerHelper(0, EngineType::ENGINE_CCS);

    EXPECT_TRUE(csHelper.isMemorySupported(MEMORY_BANK_0, 65536));
}

TEST(IsMemorySupportedCri, givenSystemMemoryWhen4KBAnd64KBAlignmentThenIsSupported) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);

    auto &csHelper = gpu->getCommandStreamerHelper(0, EngineType::ENGINE_CCS);

    EXPECT_TRUE(csHelper.isMemorySupported(MEMORY_BANK_SYSTEM, 4096));
    EXPECT_TRUE(csHelper.isMemorySupported(MEMORY_BANK_SYSTEM, 65536));
}

TEST(IsMemorySupportedCri, givenUnsupportedMemoryBankThenReturnFalse) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);

    auto &csHelper = gpu->getCommandStreamerHelper(0, EngineType::ENGINE_CCS);

    EXPECT_FALSE(csHelper.isMemorySupported(MEMORY_BANK_4, 65536));
    EXPECT_FALSE(csHelper.isMemorySupported(MEMORY_BANK_5, Page2MB::pageSize2MB));
}

struct Page2MBCriFixture : public MockAubStreamFixture, public ::testing::Test {
    void SetUp() override {
        MockAubStreamFixture::SetUp();
    }

    void TearDown() override {
        MockAubStreamFixture::TearDown();
    }
};

TEST_F(Page2MBCriFixture, givenCriWhenWriteMemoryWith2MBPagesThenCorrectHierarchyIsCreated) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);
    TEST_REQUIRES(localMemorySupportedInTests);

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress = 0x200000;

    stream.writeMemory(&ppgtt, {gfxAddress, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    // Verify hierarchy
    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);

    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);

    auto page = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page);
    EXPECT_EQ(Page2MB::pageSize2MB, page->getPageSize());
}

TEST_F(Page2MBCriFixture, givenCriWhenWalkingMemoryFor2MBPageWithCompressionThenAdditionalParamsAreApplied) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Cri);
    TEST_REQUIRES(localMemorySupportedInTests);

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint64_t gfxAddress = 0x200000;
    AllocationParams params(gfxAddress, nullptr, Page2MB::pageSize2MB, MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB);
    params.additionalParams.compressionEnabled = true;

    stream.writeMemory(&ppgtt, params);

    // Verify compression is applied to the page
    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    auto page = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page);

    EXPECT_TRUE(page->peekAllocationParams().compressionEnabled);
}
