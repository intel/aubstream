/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"
#include "aub_services.h"
#include "aubstream/product_family.h"

namespace aub_stream {

struct GpuCfl : public GpuGen9 {
    GpuCfl() {
        productFamily = ProductFamily::Cfl;
        gfxCoreFamily = CoreFamily::Gen9;
        productAbbreviation = "cfl";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Cfl;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Cfl>() {
    return std::make_unique<GpuCfl>;
}
} // namespace aub_stream
