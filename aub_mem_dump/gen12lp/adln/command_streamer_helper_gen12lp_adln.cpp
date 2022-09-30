/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

namespace aub_stream {

struct GpuAdln : public GpuGen12LP {
    GpuAdln() {
        productFamily = IGFX_ALDERLAKE_N;
        gfxCoreFamily = GEN12LP_CORE;
        productAbbreviation = "adln";
        deviceId = 0x22;
        deviceCount = 1;
    }
};

template <>
const Gpu *enableGpu<IGFX_ALDERLAKE_N>() {
    static const GpuAdln gpu;
    return &gpu;
}
} // namespace aub_stream
