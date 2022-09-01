/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "test_defaults.h"
#include "gtest/gtest.h"

#include "test.h"

using namespace aub_stream;

TEST(Gpu, gpuXeHpReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == IGFX_XE_HP_SDV);

    EXPECT_EQ(0x1d, gpu->deviceId);
}
