/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"
#include "aub_services.h"

namespace aub_stream {

struct GpuGlk : public GpuGen9 {
    GpuGlk() {
        productFamily = IGFX_GEMINILAKE;
        gfxCoreFamily = GEN9_CORE;
        productAbbreviation = "glk";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Glk;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<IGFX_GEMINILAKE>() {
    static const GpuGlk glk;
    return &glk;
}

} // namespace aub_stream
