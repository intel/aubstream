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
#ifdef SUPPORT_ICLLP
static RegisterFamily<ProductFamily::Icllp> registerFamilyIcllp;
#endif
} // namespace aub_stream
