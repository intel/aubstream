/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"
#include "aub_services.h"

namespace aub_stream {

struct GpuSkl : public GpuGen9 {
    GpuSkl() {
        productFamily = IGFX_SKYLAKE;
        gfxCoreFamily = GEN9_CORE;
        productAbbreviation = "skl";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Skl;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<IGFX_SKYLAKE>() {
    static const GpuSkl skl;
    return &skl;
}

} // namespace aub_stream
