/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {
static RegisterFamily<IGFX_XE_HP_SDV> registerFamilyXeHp;
} // namespace aub_stream
