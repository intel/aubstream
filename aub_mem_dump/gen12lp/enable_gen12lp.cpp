/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {
#ifdef SUPPORT_TGLLP
static RegisterFamily<IGFX_TIGERLAKE_LP> registerFamilyTgllp;
#endif
#ifdef SUPPORT_DG1
static RegisterFamily<IGFX_DG1> registerFamilyDg1;
#endif
#ifdef SUPPORT_ADLS
static RegisterFamily<IGFX_ALDERLAKE_S> registerFamilyAdls;
#endif
#ifdef SUPPORT_ADLN
static RegisterFamily<IGFX_ALDERLAKE_N> registerFamilyAdln;
#endif
#ifdef SUPPORT_ADLP
static RegisterFamily<IGFX_ALDERLAKE_P> registerFamilyAdlp;
#endif
} // namespace aub_stream
