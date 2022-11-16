/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aubstream/headers/product_family.h"
#include <cassert>

namespace aub_stream {
struct TestTraits {
    TestTraits() = delete;

    TestTraits(uint32_t deviceSliceCount, uint32_t deviceSubSliceCount, uint32_t deviceEuPerSubSlice)
        : deviceSliceCount(deviceSliceCount), deviceSubSliceCount(deviceSubSliceCount), deviceEuPerSubSlice(deviceEuPerSubSlice) {}

    uint32_t deviceSliceCount;
    uint32_t deviceSubSliceCount;
    uint32_t deviceEuPerSubSlice;
};

extern const TestTraits *testTraits[static_cast<uint32_t>(ProductFamily::MaxProduct)];

template <ProductFamily productFamily>
struct EnableTestTraits {
    EnableTestTraits() = delete;

    EnableTestTraits(uint64_t config) {
        static TestTraits traits(static_cast<uint32_t>((config >> 32) & 0xFFFF),
                                 static_cast<uint32_t>((config >> 16) & 0xFFFF),
                                 static_cast<uint32_t>(config & 0xFFFF));

        assert(!testTraits[static_cast<uint32_t>(productFamily)]);

        testTraits[static_cast<uint32_t>(productFamily)] = &traits;
    }
};

} // namespace aub_stream
