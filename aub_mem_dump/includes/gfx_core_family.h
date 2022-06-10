/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>

namespace aub_stream {

// This is replicated from igfxfmid.h.  Values should be the same
// but support in this project is limited to these core families
enum GFXCORE_FAMILY : uint32_t {
    UNKNOWN_CORE = 0,
    GEN8_CORE = 11,
    GEN9_CORE = 12,
    GEN10_CORE = 13,
    GEN10LP_CORE = 14,
    GEN11_CORE = 15,
    GEN11LP_CORE = 16,
    GEN12_CORE = 17,
    GEN12LP_CORE = 18,
    XE_HP_CORE = 0x0c05,
    MAX_CORE, //Max Family, for lookup table
};

} // namespace aub_stream
