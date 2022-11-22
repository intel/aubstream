/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

#include "aubstream/product_family.h"
namespace aub_stream {
#ifdef SUPPORT_PVC
static RegisterFamily<ProductFamily::Pvc> registerFamilyPvc;
#endif
} // namespace aub_stream
