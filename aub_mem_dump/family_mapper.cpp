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

std::unique_ptr<std::map<ProductFamily, std::function<std::unique_ptr<Gpu>()>>> productFamilyTable;

std::function<std::unique_ptr<Gpu>()> getGpu(ProductFamily productFamily) {
    if (productFamilyTable.get()->find(productFamily) != productFamilyTable.get()->end()) {
        return (*productFamilyTable.get())[productFamily];
    }
    return {};
}
} // namespace aub_stream
