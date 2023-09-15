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

struct GpuSkl : public GpuGen9 {
    GpuSkl() {
        productFamily = ProductFamily::Skl;
        gfxCoreFamily = CoreFamily::Gen9;
        productAbbreviation = "skl";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Skl;
        deviceCount = 1;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Skl>() {
    return std::make_unique<GpuSkl>;
}
} // namespace aub_stream
