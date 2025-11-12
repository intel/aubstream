/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "aub_mem_dump/xe3p_core/command_streamer_helper_xe3p_core.h"
#include "test_defaults.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

struct MockPageTableMemory : PageTableMemory {
    using PageTableMemory::additionalAllocParams;
    using PageTableMemory::PageTableMemory;
};

TEST(PageTableTestsXe3pCore, givenPageTableInLocalMemoryWhenCallingGetEntryValueThenCorrectBitsAreSet) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe3pCore);

    uint64_t physicalAddress = 0x20000;
    MockPageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_0);

    // cached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = false;

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex0;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex9;
        auto expectedBaseBits = physicalAddress |
                                toBitValue(PpgttEntryBits::localMemoryBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled | expectedBaseBits, pageTable.getEntryValue());

        params.compressionEnabled = true;
        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled | expectedBaseBits, pageTable.getEntryValue());
    }

    // uncached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = true;

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex3;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex12;
        auto expectedBaseBits = physicalAddress |
                                toBitValue(PpgttEntryBits::localMemoryBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled | expectedBaseBits, pageTable.getEntryValue());

        params.compressionEnabled = true;
        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled | expectedBaseBits, pageTable.getEntryValue());
    }
}

TEST(PageTableTestsXe3pCore, givenPageTableInSystemMemoryWhenCallingGetEntryValueThenCorrectBitsAreSet) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe3pCore);

    uint64_t physicalAddress = 0x20000;
    MockPageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_SYSTEM);

    // cached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = false;

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex0;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex9;
        auto expectedBaseBits = physicalAddress |
                                toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled | expectedBaseBits, pageTable.getEntryValue());

        params.compressionEnabled = true;
        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled | expectedBaseBits, pageTable.getEntryValue());
    }

    // uncached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = true;

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex3;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe3pCore::patIndex12;
        auto expectedBaseBits = physicalAddress |
                                toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionDisabled | expectedBaseBits, pageTable.getEntryValue());

        params.compressionEnabled = true;
        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBitsWithCompressionEnabled | expectedBaseBits, pageTable.getEntryValue());
    }
}
