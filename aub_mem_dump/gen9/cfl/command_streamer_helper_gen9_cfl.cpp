/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"
#include "aub_services.h"

namespace aub_stream {

struct GpuCfl : public GpuGen9 {
    GpuCfl() {
        productFamily = IGFX_COFFEELAKE;
        gfxCoreFamily = GEN9_CORE;
        productAbbreviation = "cfl";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Cfl;
        deviceCount = 1;
    }
};
template <>
const Gpu *enableGpu<IGFX_COFFEELAKE>() {
    static const GpuCfl cfl;
    return &cfl;
}

} // namespace aub_stream
