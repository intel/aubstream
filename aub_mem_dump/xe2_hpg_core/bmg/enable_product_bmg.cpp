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
static RegisterFamily<ProductFamily::Bmg> registerFamilyBmg;
} // namespace aub_stream