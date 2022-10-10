/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {
#ifdef SUPPORT_DG2
static RegisterFamily<IGFX_DG2> registerFamilyDg2;
#endif
} // namespace aub_stream
