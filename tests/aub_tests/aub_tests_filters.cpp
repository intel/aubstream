/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gpu.h"
#include "gfx_core_family.h"
#include "aubstream/product_family.h"
#include <string>

using namespace aub_stream;
std::string getTestFilter(const Gpu &gpu) {
    std::string filter = "";
    if (gpu.gfxCoreFamily <= CoreFamily::Gen11) {
        filter += "*NotEqual*";
    }

    if (gpu.gfxCoreFamily == CoreFamily::Gen8) {
        filter += ":*SimpleBatchBuffer.vecs*:*SimpleBatchBuffer.vcs*";
    }

    return filter;
}
