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

TEST(Gpu, gpuPtlReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Ptl);

    EXPECT_EQ(51u, gpu->deviceId);
}

TEST(Ptl, allocatePML5) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Ptl);
    PhysicalAddressAllocatorSimple allocator;

    auto ppgtt = std::unique_ptr<PageTable>(gpu->allocatePPGTT(&allocator, MEMORY_BANK_0, maxNBitValue(57)));
    EXPECT_EQ(57u, ppgtt->getNumAddressBits());
}
