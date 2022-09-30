/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {
static RegisterFamily<IGFX_TIGERLAKE_LP> registerFamilyTgllp;
static RegisterFamily<IGFX_DG1> registerFamilyDg1;
static RegisterFamily<IGFX_ALDERLAKE_S> registerFamilyAdls;
static RegisterFamily<IGFX_ALDERLAKE_N> registerFamilyAdln;
static RegisterFamily<IGFX_ALDERLAKE_P> registerFamilyAdlp;
} // namespace aub_stream
