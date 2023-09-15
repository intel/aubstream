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

std::map<ProductFamily, std::function<std::unique_ptr<Gpu>()>> *productFamilyTable = nullptr;

std::function<std::unique_ptr<Gpu>()> getGpu(ProductFamily productFamily) {
    if ((*productFamilyTable).find(productFamily) != (*productFamilyTable).end()) {
        return (*productFamilyTable)[productFamily];
    }
    return {};
}
} // namespace aub_stream
