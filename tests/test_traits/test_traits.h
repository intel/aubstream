/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aubstream/product_family.h"
#include "aub_mem_dump/gpu.h"
#include "tests/test_defaults.h"
#include <cassert>
#include <functional>

namespace aub_stream {

struct TestTraits {
    TestTraits() = delete;

    TestTraits(uint32_t deviceSliceCount, uint32_t deviceSubSliceCount, uint32_t deviceEuPerSubSlice, const std::function<std::string(const TestTraits *, const GpuDescriptor &desc)> &&func)
        : deviceSliceCount(deviceSliceCount), deviceSubSliceCount(deviceSubSliceCount), deviceEuPerSubSlice(deviceEuPerSubSlice), getAubConfig(func) {}

    uint32_t deviceSliceCount;
    uint32_t deviceSubSliceCount;
    uint32_t deviceEuPerSubSlice;
    std::function<std::string(const TestTraits *, const GpuDescriptor &)> getAubConfig;
};

extern const TestTraits *testTraits[static_cast<uint32_t>(ProductFamily::MaxProduct)];

template <ProductFamily productFamily, auto getConfig = getAubConfig>
struct EnableTestTraits {
    EnableTestTraits() = delete;

    EnableTestTraits(uint64_t config) {
        static TestTraits traits(static_cast<uint32_t>((config >> 32) & 0xFFFF),
                                 static_cast<uint32_t>((config >> 16) & 0xFFFF),
                                 static_cast<uint32_t>(config & 0xFFFF),
                                 getConfig);

        assert(!testTraits[static_cast<uint32_t>(productFamily)]);

        testTraits[static_cast<uint32_t>(productFamily)] = &traits;
    }
};

} // namespace aub_stream
