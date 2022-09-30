/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST(Gpu, gpuDg2ReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == IGFX_DG2);

    EXPECT_EQ(0x24, gpu->deviceId);
}

TEST(Gpu, givenDg2WhenInitializeDefaultMemoryPoolsThenNotInitializeFlatCcsBaseAddressPtr) {
    TEST_REQUIRES(gpu->productFamily == IGFX_DG2);
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

    gpu->initializeDefaultMemoryPools(stream, numDevices, perDeviceHbmSize);
}

TEST(Gpu, givenDg2AndFileStreamWhenInitializeDefaultMemoryPoolsThenFlatCcsBaseAddressIsProgrammed) {
    TEST_REQUIRES(gpu->productFamily == IGFX_DG2);

    MockAubFileStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x00004910, _)).Times(0);

    gpu->initializeDefaultMemoryPools(stream, 1, 1);
}

TEST(Gpu, givenDg2AndTbxStreamInitializeDefaultMemoryPoolsThenFlatCcsBaseAddressIsProgrammed) {
    TEST_REQUIRES(gpu->productFamily == IGFX_DG2);

    MockTbxStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x00004910, _)).Times(1);

    gpu->initializeDefaultMemoryPools(stream, 1, 1);
}
TEST(Gpu, givenDg2WhenCheckEngineSuportThenAllExpectedEnginesAreSupported) {
    TEST_REQUIRES(gpu->productFamily == IGFX_DG2);
    EXPECT_TRUE(gpu->isEngineSupported(ENGINE_RCS));
    EXPECT_TRUE(gpu->isEngineSupported(ENGINE_BCS));
    EXPECT_TRUE(gpu->isEngineSupported(ENGINE_CCS));
    EXPECT_TRUE(gpu->isEngineSupported(ENGINE_CCS1));
    EXPECT_TRUE(gpu->isEngineSupported(ENGINE_CCS2));
    EXPECT_TRUE(gpu->isEngineSupported(ENGINE_CCS3));
}