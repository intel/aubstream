/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen11/command_streamer_helper_gen11.h"
#include "aub_services.h"

namespace aub_stream {

struct GpuIcllp : public GpuGen11 {
    GpuIcllp() {
        productFamily = IGFX_ICELAKE_LP;
        gfxCoreFamily = GEN11_CORE;
        productAbbreviation = "icllp";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Icllp;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<IGFX_ICELAKE_LP>() {
    static const GpuIcllp icllp;
    return &icllp;
}

} // namespace aub_stream
