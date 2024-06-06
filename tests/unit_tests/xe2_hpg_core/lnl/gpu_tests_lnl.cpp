/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "test_defaults.h"
#include "gtest/gtest.h"
#include "aub_mem_dump/memory_banks.h"

#include "test.h"

using namespace aub_stream;

TEST(Gpu, gpuLnlReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Lnl);

    EXPECT_EQ(45u, gpu->deviceId);
}

TEST(Lnl, allocatePML5) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Lnl);
    PhysicalAddressAllocatorSimple allocator;

    auto ppgtt = std::unique_ptr<PageTable>(gpu->allocatePPGTT(&allocator, MEMORY_BANK_0, maxNBitValue(57)));
    EXPECT_EQ(57u, ppgtt->getNumAddressBits());
}
