/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tests/test_traits/test_traits.h"
#include "tests/test_defaults.h"
#include <sstream>

namespace aub_stream {
namespace {
std::string getConfigCri(const TestTraits *traits, const GpuDescriptor &desc) {
    std::stringstream strConfig;

    strConfig << desc.deviceCount
              << "tx";

    uint32_t xeCuCount = 1;
    uint32_t sliceCount = traits->deviceSliceCount;
    if (sliceCount > 4) {
        xeCuCount = sliceCount / 4;
        sliceCount = sliceCount / xeCuCount;
    }

    strConfig << xeCuCount
              << "x"
              << sliceCount
              << "x"
              << traits->deviceSubSliceCount
              << "x"
              << traits->deviceEuPerSubSlice;

    return strConfig.str();
};
EnableTestTraits<ProductFamily::Cri, getConfigCri> enableCri(CRI_CONFIG);
} // namespace
} // namespace aub_stream
