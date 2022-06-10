/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "family_mapper.h"
#include "gpu.h"
#include <cassert>

namespace aub_stream {

const Gpu *gpuCoreFamilyTable[MAX_CORE] = {};
std::map<PRODUCT_FAMILY, const Gpu *> *productFamilyTable = nullptr;

const Gpu *getGpu(PRODUCT_FAMILY productFamily) {
    const Gpu *gpu = nullptr;
    if ((*productFamilyTable).find(productFamily) != (*productFamilyTable).end()) {
        gpu = (*productFamilyTable)[productFamily];
    }
    return gpu;
}

CommandStreamerHelper &getCommandStreamerHelper(PRODUCT_FAMILY productFamily, uint32_t device, EngineType engine) {
    auto gpu = getGpu(productFamily);
    assert(gpu);

    return gpu->getCommandStreamerHelper(device, engine);
}

} // namespace aub_stream
