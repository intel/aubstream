/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {
#ifdef SUPPORT_BDW
static RegisterFamily<IGFX_BROADWELL> registerFamilyBdw;
#endif
} // namespace aub_stream
