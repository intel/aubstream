/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

#include "aubstream/product_family.h"
namespace aub_stream {

struct GpuAdlp : public GpuGen12LP {
    GpuAdlp() {
        productFamily = ProductFamily::Adlp;
        gfxCoreFamily = CoreFamily::Gen12lp;
        productAbbreviation = "adlp";
        deviceId = 0x22;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Adlp>() {
    return std::make_unique<GpuAdlp>;
}
} // namespace aub_stream
