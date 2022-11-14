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
#ifdef SUPPORT_TGLLP
static RegisterFamily<ProductFamily::Tgllp> registerFamilyTgllp;
#endif
#ifdef SUPPORT_DG1
static RegisterFamily<ProductFamily::Dg1> registerFamilyDg1;
#endif
#ifdef SUPPORT_ADLS
static RegisterFamily<ProductFamily::Adls> registerFamilyAdls;
#endif
#ifdef SUPPORT_ADLN
static RegisterFamily<ProductFamily::Adln> registerFamilyAdln;
#endif
#ifdef SUPPORT_ADLP
static RegisterFamily<ProductFamily::Adlp> registerFamilyAdlp;
#endif
} // namespace aub_stream
