/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "family_mapper.h"
#include "gpu.h"
#include <cassert>

namespace aub_stream {

std::map<ProductFamily, const Gpu *> *productFamilyTable = nullptr;

const Gpu *getGpu(ProductFamily productFamily) {
    const Gpu *gpu = nullptr;
    if ((*productFamilyTable).find(productFamily) != (*productFamilyTable).end()) {
        gpu = (*productFamilyTable)[productFamily];
    }
    return gpu;
}

CommandStreamerHelper &getCommandStreamerHelper(ProductFamily productFamily, uint32_t device, EngineType engine) {
    auto gpu = getGpu(productFamily);
    assert(gpu);

    return gpu->getCommandStreamerHelper(device, engine);
}

} // namespace aub_stream
