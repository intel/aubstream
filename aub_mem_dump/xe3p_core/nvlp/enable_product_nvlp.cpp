/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aubstream/product_family.h"

namespace aub_stream {
static RegisterFamily<ProductFamily::Nvlp> registerFamilyNvlp;
} // namespace aub_stream
