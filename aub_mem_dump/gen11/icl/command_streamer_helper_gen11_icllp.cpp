/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen11/command_streamer_helper_gen11.h"
#include "aub_services.h"
#include "aubstream/headers/product_family.h"

namespace aub_stream {

struct GpuIcllp : public GpuGen11 {
    GpuIcllp() {
        productFamily = ProductFamily::Icllp;
        gfxCoreFamily = CoreFamily::Gen11;
        productAbbreviation = "icllp";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Icllp;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<ProductFamily::Icllp>() {
    static const GpuIcllp icllp;
    return &icllp;
}

} // namespace aub_stream
