/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aubstream/headers/product_family.h"

namespace aub_stream {
#ifdef SUPPORT_XE_HP_SDV
static RegisterFamily<ProductFamily::XeHpSdv> registerFamilyXeHp;
#endif
} // namespace aub_stream
