/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "gfx_core_family.h"
#include "aubstream/engine_node.h"
#include <map>

namespace aub_stream {

struct CommandStreamerHelper;
struct Gpu;
enum class ProductFamily : uint32_t;

// Table of HW family specific Gpus
extern std::map<ProductFamily, const Gpu *> *productFamilyTable;

// Helper method to access a productFamily Gpu
template <ProductFamily>
const Gpu *enableGpu();

// Helper to register product families
template <ProductFamily productFamily>
struct RegisterFamily {
    RegisterFamily() {
        auto gpu = enableGpu<productFamily>();
        if (!productFamilyTable) {
            productFamilyTable = new std::map<ProductFamily, const Gpu *>;
        }
        (*productFamilyTable)[productFamily] = gpu;
    }
};

// Main accessor to get a Gpu
const Gpu *getGpu(ProductFamily productFamily);

// Main accessor to get a CommandStreamerHelper
CommandStreamerHelper &getCommandStreamerHelper(ProductFamily productFamily, uint32_t device, EngineType engine);

} // namespace aub_stream
