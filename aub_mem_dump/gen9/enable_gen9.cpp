/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {

static RegisterFamily<IGFX_SKYLAKE> registerFamilySkl;
static RegisterFamily<IGFX_KABYLAKE> registerFamilyKbl;
static RegisterFamily<IGFX_GEMINILAKE> registerFamilyGlk;
static RegisterFamily<IGFX_BROXTON> registerFamilyBxt;
static RegisterFamily<IGFX_COFFEELAKE> registerFamilyCfl;

} // namespace aub_stream