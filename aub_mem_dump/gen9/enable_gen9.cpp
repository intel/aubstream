/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "product_family.h"

namespace aub_stream {

#ifdef SUPPORT_SKL
static RegisterFamily<ProductFamily::Skl> registerFamilySkl;
#endif
#ifdef SUPPORT_KBL
static RegisterFamily<ProductFamily::Kbl> registerFamilyKbl;
#endif
#ifdef SUPPORT_GLK
static RegisterFamily<ProductFamily::Glk> registerFamilyGlk;
#endif
#ifdef SUPPORT_BXT
static RegisterFamily<ProductFamily::Bxt> registerFamilyBxt;
#endif
#ifdef SUPPORT_CFL
static RegisterFamily<ProductFamily::Cfl> registerFamilyCfl;
#endif

} // namespace aub_stream
