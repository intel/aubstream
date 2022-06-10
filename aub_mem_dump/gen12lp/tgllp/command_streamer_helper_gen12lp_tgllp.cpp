/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

namespace aub_stream {

struct GpuTgllp : public GpuGen12LP {
    GpuTgllp() {
        productFamily = IGFX_TIGERLAKE_LP;
        gfxCoreFamily = GEN12LP_CORE;
        productAbbreviation = "tgllp";
        deviceId = 0x16;
        deviceCount = 1;
    }
};

template <>
const Gpu *enableGpu<IGFX_TIGERLAKE_LP>() {
    static const GpuTgllp gpu;
    return &gpu;
}

} // namespace aub_stream
