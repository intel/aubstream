/*
 * Copyright (C) 2022 Intel Corporation
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
const Gpu *enableGpu<ProductFamily::Adlp>() {
    static const GpuAdlp gpu;
    return &gpu;
}
} // namespace aub_stream
