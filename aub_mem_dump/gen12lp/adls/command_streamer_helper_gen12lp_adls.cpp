/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"
#include "aubstream/product_family.h"

namespace aub_stream {

struct GpuAdls : public GpuGen12LP {
    GpuAdls() {
        productFamily = ProductFamily::Adls;
        gfxCoreFamily = CoreFamily::Gen12lp;
        productAbbreviation = "adls";
        deviceId = 0x25;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Adls>() {
    return std::make_unique<GpuAdls>;
}
} // namespace aub_stream
