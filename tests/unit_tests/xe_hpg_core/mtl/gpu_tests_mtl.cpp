/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "test.h"

#include "aub_mem_dump/xe_hpg_core/command_streamer_helper_xe_hpg_core.h"

#include "aubstream/engine_node.h"
#include "tests/unit_tests/command_stream_helper_tests.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

struct MockPageTableMemory : PageTableMemory {
    using PageTableMemory::additionalAllocParams;
    using PageTableMemory::PageTableMemory;
};

TEST(Gpu, givenDeviceIdWhenMtlGpuIsInitializeThenReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Mtl);

    EXPECT_EQ(42, gpu->deviceId);
}

TEST(Gpu, givenMtlWhenInitializeDefaultMemoryPoolsThenFileStreamNotInitializeFlatCcsBaseAddress) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Mtl);

    MockAubFileStream stream;
    gpu->stolenMemory = StolenMemory::CreateStolenMemory(false, 1, 1, 1 * MB);
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x00004910, _)).Times(0);

    gpu->initializeDefaultMemoryPools(stream, 1, 1);
}

TEST(Gpu, givenMtlWhenInitializingGlobalMmiosThenProgramPatIndex) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Mtl);

    MockAubFileStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));

    EXPECT_CALL(stream, writeMMIO(0x00004800, 0x0)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00004804, 0x4)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00004808, 0xC)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000480C, 0x2)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00004810, 0x3)).Times(1);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0);
}

TEST(Gpu, givenMtlAndTbxStreamWhenInitializeDefaultMemoryPoolsThenNotInitializeFlatCcsBaseAddress) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Mtl);

    MockTbxStream stream;
    gpu->stolenMemory = StolenMemory::CreateStolenMemory(false, 1, 1, 1 * MB);
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x00004910, _)).Times(0);

    gpu->initializeDefaultMemoryPools(stream, 1, 1);
}

TEST(Gpu, givenMtlWhenInitializeDefaultMemoryPoolsThenNotInitializeFlatCcsBaseAddressPtr) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Mtl);
    constexpr uint32_t mmioDeviceOffset = 16 * MB;
    constexpr uint32_t numDevices = 4;
    constexpr uint64_t perDeviceHbmSize = 8llu * GB;

    MockTbxStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));

    for (uint32_t i = 0; i > numDevices; i++) {
        uint64_t flatCcsBaseAddress = perDeviceHbmSize * (i + 1);            // device local memory ending
        flatCcsBaseAddress -= 9 * MB;                                        // wopcm and ggtt
        flatCcsBaseAddress -= (perDeviceHbmSize / 256);                      // flat_ccs size
        uint32_t mmioValue = static_cast<uint32_t>(flatCcsBaseAddress >> 8); // [8:31] base ptr
        mmioValue |= 1;                                                      // [0] enable bit

        EXPECT_CALL(stream, writeMMIO((i * mmioDeviceOffset) + 0x00004910, mmioValue)).Times(0);
    }

    gpu->stolenMemory = StolenMemory::CreateStolenMemory(false, 1, 1, 1 * MB);
    gpu->initializeDefaultMemoryPools(stream, numDevices, perDeviceHbmSize);
}

TEST(PageTableTestsMtl, givenPageTableInSystemMemoryWhenCallingGetEntryValueThenCorrectBitsAreSet) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Mtl);

    uint64_t physicalAddress = 0x20000;
    MockPageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_SYSTEM);

    // cached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = false;

        auto expectedExtraBits = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXeHpgCore::patIndex0;
        auto expectedBaseBits = physicalAddress |
                                toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBits, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBits | expectedBaseBits, pageTable.getEntryValue());
    }

    // uncached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = true;

        auto expectedExtraBits = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXeHpgCore::patIndex2;
        auto expectedBaseBits = physicalAddress |
                                toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        pageTable.additionalAllocParams = params;
        EXPECT_EQ(expectedExtraBits, gpu->getPPGTTExtraEntryBits(params));
        EXPECT_EQ(expectedExtraBits | expectedBaseBits, pageTable.getEntryValue());
    }
}
