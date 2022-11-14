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

struct GpuBxt : public GpuGen9 {
    GpuBxt() {
        productFamily = ProductFamily::Bxt;
        gfxCoreFamily = CoreFamily::Gen9;
        productAbbreviation = "bxt";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Bxt;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<ProductFamily::Bxt>() {
    static const GpuBxt bxt;
    return &bxt;
}

} // namespace aub_stream
