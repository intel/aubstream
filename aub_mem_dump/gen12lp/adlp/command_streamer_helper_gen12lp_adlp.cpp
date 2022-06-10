/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

namespace aub_stream {

struct GpuAdlp : public GpuGen12LP {
    GpuAdlp() {
        productFamily = IGFX_ALDERLAKE_P;
        gfxCoreFamily = GEN12LP_CORE;
        productAbbreviation = "adlp";
        deviceId = 0x22;
        deviceCount = 1;
    }
};

template <>
const Gpu *enableGpu<IGFX_ALDERLAKE_P>() {
    static const GpuAdlp gpu;
    return &gpu;
}
} // namespace aub_stream
