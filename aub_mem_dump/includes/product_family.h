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
enum PRODUCT_FAMILY : uint32_t {
    IGFX_UNKNOWN = 0,
    IGFX_BROADWELL = 16,
    IGFX_CHERRYVIEW,
    IGFX_SKYLAKE,
    IGFX_KABYLAKE,
    IGFX_COFFEELAKE,
    IGFX_WILLOWVIEW,
    IGFX_BROXTON,
    IGFX_GEMINILAKE,
    IGFX_CANNONLAKE,
    IGFX_ICELAKE,
    IGFX_ICELAKE_LP,
    IGFX_LAKEFIELD,
    IGFX_JASPERLAKE,
    IGFX_ELKHARTLAKE = IGFX_JASPERLAKE,
    IGFX_TIGERLAKE_LP,
    IGFX_ROCKETLAKE,
    IGFX_ALDERLAKE_S,
    IGFX_ALDERLAKE_P,
    IGFX_DG1 = 1210,
    IGFX_XE_HP_SDV = 1250,
    IGFX_MAX_PRODUCT,
};

} // namespace aub_stream
