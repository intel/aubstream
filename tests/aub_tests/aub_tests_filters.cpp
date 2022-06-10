/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gpu.h"
#include "gfx_core_family.h"
#include "product_family.h"
#include <string>

using namespace aub_stream;
std::string getTestFilter(const Gpu &gpu) {
    std::string filter = "";
    if (gpu.gfxCoreFamily <= GFXCORE_FAMILY::GEN11LP_CORE) {
        filter += "*NotEqual*";
    }

    if (gpu.gfxCoreFamily == GFXCORE_FAMILY::GEN8_CORE) {
        filter += ":*SimpleBatchBuffer.vecs*:*SimpleBatchBuffer.vcs*";
    }

    return filter;
}