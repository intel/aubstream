/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include "test.h"

using namespace aub_stream;

TEST(Gpu, gpuDg1ReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Dg1);

    EXPECT_EQ(0x1e, gpu->deviceId);
}

TEST(Gpu, dg1GivenOneDeviceSetMemoryBankSizeOnlyDefinesOneBank) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Dg1);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, 0x00000201, 0xffffffff)).Times(1);

    auto deviceCount = 1;
    auto memoryBankSize = 2 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}
