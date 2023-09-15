/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

#include "aubstream/product_family.h"

namespace aub_stream {

struct GpuTgllp : public GpuGen12LP {
    GpuTgllp() {
        productFamily = ProductFamily::Tgllp;
        gfxCoreFamily = CoreFamily::Gen12lp;
        productAbbreviation = "tgllp";
        deviceId = 0x16;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Tgllp>() {
    return std::make_unique<GpuTgllp>;
}

} // namespace aub_stream
