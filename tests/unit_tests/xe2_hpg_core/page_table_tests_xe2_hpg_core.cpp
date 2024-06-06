/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "aub_mem_dump/xe2_hpg_core/command_streamer_helper_xe2_hpg_core.h"
#include "test_defaults.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

struct MockPageTableMemory : PageTableMemory {
    using PageTableMemory::additionalAllocParams;
    using PageTableMemory::PageTableMemory;
};

TEST(PageTableTestsXe2HpgCore, givenPageTableInLocalMemoryWhenCallingGetEntryValueThenCorrectBitsAreSet) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe2HpgCore);

    uint64_t physicalAddress = 0x20000;
    MockPageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_0);

    // cached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = false;

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex0;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex9;
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

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex2;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex12;
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

TEST(PageTableTestsXe2HpgCore, givenPageTableInSystemMemoryWhenCallingGetEntryValueThenCorrectBitsAreSet) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe2HpgCore);

    uint64_t physicalAddress = 0x20000;
    MockPageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_SYSTEM);

    // cached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = false;

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex0;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex9;
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

        auto expectedExtraBitsWithCompressionDisabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex2;
        auto expectedExtraBitsWithCompressionEnabled = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXe2HpgCore::patIndex12;
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

TEST(Gpu, givenXe2HpgWhenInitializingGlobalMmiosThenProgramPatIndex) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe2HpgCore);

    MockAubFileStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x4800, 0b0000'0000'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4804, 0b0000'0000'1110)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4808, 0b0000'0000'1111)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x480C, 0b0000'0011'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4810, 0b0000'0011'0010)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4814, 0b0000'0011'1110)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4818, 0b0100'0001'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x481C, 0b0000'0011'0011)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4848, 0b0000'0011'0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x484C, 0b0010'0000'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4850, 0b0010'0011'0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4854, 0b0110'0001'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4858, 0b0010'0011'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x485C, 0b0000'0000'0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4860, 0b0010'0000'0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4864, 0b0110'0001'0100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4878, 0b0000'0100'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x487c, 0b0010'0100'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4880, 0b0000'0100'1110)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4884, 0b0000'0100'1111)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4888, 0b0000'1000'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x488C, 0b0010'1000'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4890, 0b0000'1000'1110)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4894, 0b0000'1000'1111)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4898, 0b0000'1100'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x489C, 0b0010'1100'1100)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x48A0, 0b0000'1100'1110)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x48A4, 0b0000'1100'1111)).Times(1);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0);
}
