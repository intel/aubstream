/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"
#include "aub_services.h"
#include "product_family.h"

namespace aub_stream {

struct GpuGlk : public GpuGen9 {
    GpuGlk() {
        productFamily = ProductFamily::Glk;
        gfxCoreFamily = CoreFamily::Gen9;
        productAbbreviation = "glk";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Glk;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<ProductFamily::Glk>() {
    static const GpuGlk glk;
    return &glk;
}

} // namespace aub_stream
