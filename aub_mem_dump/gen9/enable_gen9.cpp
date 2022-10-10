/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {

#ifdef SUPPORT_SKL
static RegisterFamily<IGFX_SKYLAKE> registerFamilySkl;
#endif
#ifdef SUPPORT_KBL
static RegisterFamily<IGFX_KABYLAKE> registerFamilyKbl;
#endif
#ifdef SUPPORT_GLK
static RegisterFamily<IGFX_GEMINILAKE> registerFamilyGlk;
#endif
#ifdef SUPPORT_BXT
static RegisterFamily<IGFX_BROXTON> registerFamilyBxt;
#endif
#ifdef SUPPORT_CFL
static RegisterFamily<IGFX_COFFEELAKE> registerFamilyCfl;
#endif

} // namespace aub_stream
