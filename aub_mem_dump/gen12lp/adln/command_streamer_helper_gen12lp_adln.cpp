/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aubstream/product_family.h"
#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

namespace aub_stream {

struct GpuAdln : public GpuGen12LP {
    GpuAdln() {
        productFamily = ProductFamily::Adln;
        gfxCoreFamily = CoreFamily::Gen12lp;
        productAbbreviation = "adln";
        deviceId = 0x22;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Adln>() {
    return std::make_unique<GpuAdln>;
}
} // namespace aub_stream
