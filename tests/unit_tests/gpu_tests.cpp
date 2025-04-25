/*
 * Copyright (C) 2022-2025 Intel Corporation
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
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Tgllp);

    EXPECT_EQ(0x16, gpu->deviceId);
}

TEST(Gpu, gpuAdlpReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Adlp);

    EXPECT_EQ(0x22, gpu->deviceId);
}

TEST(Gpu, gpuAdlsReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Adls);

    EXPECT_EQ(0x25, gpu->deviceId);
}

TEST(Gpu, initializeGlobalMMIOIsWithinDeviceMmioRange) {
    VerifyMmioAubStream stream(0, gpu->deviceCount * 16 * MB);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0u);
}

TEST(Gpu, gen12lpGivenOneIntegratedDeviceSetMemoryBankSizeOnlyDefinesOneBank) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Gen12lp && gpu->productFamily != ProductFamily::Dg1);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, 0x00000000)).Times(1);

    auto deviceCount = 1;
    auto memoryBankSize = 0 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}

using GpuForStolenTest = ::testing::Test;
HWTEST_F(GpuForStolenTest, isValidDataStolenMemorySizeForVariousInputCoreBelowEqualXeHpc, HwMatcher::coreBelowEqualXeHpc) {
    auto gpu = createGpuFunc();
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x123), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x123000), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x12300), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x1230000), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x02300000), true);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x02300001), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0xf2300001), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0xf2300000), false);
}

HWTEST_F(GpuForStolenTest, isValidDataStolenMemorySizeForVariousInputCoreAboveXeHpc, HwMatcher::Not<HwMatcher::coreBelowEqualXeHpc>) {
    auto gpu = createGpuFunc();
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x123), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x123000), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x12300), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x1230000), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x02300000), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0x02300001), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0xf2300001), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(0xf2300000), false);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(4 * 1024 * 1024), true);
    EXPECT_EQ(gpu->isValidDataStolenMemorySize(64 * 1024 * 1024), true);
}