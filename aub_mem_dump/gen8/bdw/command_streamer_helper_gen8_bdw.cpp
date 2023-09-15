/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen8/command_streamer_helper_gen8.h"
#include "aub_services.h"
#include "aubstream/product_family.h"

namespace aub_stream {

struct GpuBdw : public GpuGen8 {
    GpuBdw() {
        productFamily = ProductFamily::Bdw;
        gfxCoreFamily = CoreFamily::Gen8;
        productAbbreviation = "bdw";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Bdw;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Bdw>() {
    return std::make_unique<GpuBdw>;
}
} // namespace aub_stream
