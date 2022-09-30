/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;

TEST(Gpu, gpuAdlnReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == IGFX_ALDERLAKE_N);

    EXPECT_EQ(0x22, gpu->deviceId);
}
