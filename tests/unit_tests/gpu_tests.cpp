/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST(Gpu, gpuTgllpReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == IGFX_TIGERLAKE_LP);

    EXPECT_EQ(0x16, gpu->deviceId);
}

TEST(Gpu, gpuAdlpReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == IGFX_ALDERLAKE_P);

    EXPECT_EQ(0x22, gpu->deviceId);
}

TEST(Gpu, gpuAdlsReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == IGFX_ALDERLAKE_S);

    EXPECT_EQ(0x25, gpu->deviceId);
}

TEST(Gpu, initializeGlobalMMIOIsWithinDeviceMmioRange) {
    VerifyMmioAubStream stream(0, gpu->deviceCount * 16 * MB);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0u);
}

TEST(Gpu, initializeGlobalMMIOAlsoWritesMmioListInjected) {
    MMIOListInjected.push_back(MMIOPair(0xE48C, 0x20002));
    MMIOListInjected.push_back(MMIOPair(0xE4F0, 0x20002));

    MockAubStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0xE48C, 0x20002)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0xE4F0, 0x20002)).Times(1);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0u);

    MMIOListInjected.resize(0);
}

TEST(Gpu, gen12lpGivenOneIntegratedDeviceSetMemoryBankSizeOnlyDefinesOneBank) {
    TEST_REQUIRES(gpu->gfxCoreFamily == GEN12LP_CORE && gpu->productFamily != IGFX_DG1);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, 0x00000000)).Times(1);

    auto deviceCount = 1;
    auto memoryBankSize = 0 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}
