/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>
#include "aubstream/product_family.h"

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
    IGFX_TIGERLAKE_LP = 33,
    IGFX_ROCKETLAKE = 35,
    IGFX_ALDERLAKE_S = 36,
    IGFX_ALDERLAKE_P = 37,
    IGFX_ALDERLAKE_N = 38,
    IGFX_DG1 = 1210,
    IGFX_XE_HP_SDV = 1250,
    IGFX_DG2 = 1270,
    IGFX_PVC = 1271,
    IGFX_MAX_PRODUCT,
};

inline ProductFamily getProductFamilyType(PRODUCT_FAMILY gfxProductFamily) {
    switch (gfxProductFamily) {
    case IGFX_BROADWELL:
        return ProductFamily::Bdw;
    case IGFX_SKYLAKE:
        return ProductFamily::Skl;
    case IGFX_KABYLAKE:
        return ProductFamily::Kbl;
    case IGFX_COFFEELAKE:
        return ProductFamily::Cfl;
    case IGFX_BROXTON:
        return ProductFamily::Bxt;
    case IGFX_GEMINILAKE:
        return ProductFamily::Glk;
    case IGFX_ICELAKE_LP:
        return ProductFamily::Icllp;
    case IGFX_TIGERLAKE_LP:
        return ProductFamily::Tgllp;
    case IGFX_ALDERLAKE_S:
        return ProductFamily::Adls;
    case IGFX_ALDERLAKE_P:
        return ProductFamily::Adlp;
    case IGFX_ALDERLAKE_N:
        return ProductFamily::Adln;
    case IGFX_DG1:
        return ProductFamily::Dg1;
    case IGFX_XE_HP_SDV:
        return ProductFamily::XeHpSdv;
    case IGFX_DG2:
        return ProductFamily::Dg2;
    case IGFX_PVC:
        return ProductFamily::Pvc;
    default:
        return ProductFamily::MaxProduct;
    }
}
} // namespace aub_stream
