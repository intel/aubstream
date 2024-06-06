/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

#include "aubstream/product_family.h"
namespace aub_stream {
#ifdef SUPPORT_BMG
static RegisterFamily<ProductFamily::Bmg> registerFamilyBmg;
#endif
#ifdef SUPPORT_LNL
static RegisterFamily<ProductFamily::Lnl> registerFamilyLnl;
#endif
} // namespace aub_stream
