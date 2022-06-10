/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen8/command_streamer_helper_gen8.h"
#include "aub_services.h"

namespace aub_stream {

struct GpuBdw : public GpuGen8 {
    GpuBdw() {
        productFamily = IGFX_BROADWELL;
        gfxCoreFamily = GEN8_CORE;
        productAbbreviation = "bdw";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Bdw;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<IGFX_BROADWELL>() {
    static const GpuBdw bdw;
    return &bdw;
}

} // namespace aub_stream
