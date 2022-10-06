/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "gfx_core_family.h"
#include "aubstream/headers/engine_node.h"
#include "product_family.h"
#include <map>

namespace aub_stream {

struct CommandStreamerHelper;
struct Gpu;

// Table of HW family specific Gpus
extern const Gpu *gpuCoreFamilyTable[MAX_CORE];
extern std::map<PRODUCT_FAMILY, const Gpu *> *productFamilyTable;

// Helper method to access a productFamily Gpu
template <PRODUCT_FAMILY>
const Gpu *enableGpu();

// Helper to register product families
template <PRODUCT_FAMILY productFamily>
struct RegisterFamily {
    RegisterFamily() {
        auto gpu = enableGpu<productFamily>();
        gpuCoreFamilyTable[gpu->gfxCoreFamily] = gpu;
        if (!productFamilyTable) {
            productFamilyTable = new std::map<PRODUCT_FAMILY, const Gpu *>;
        }
        (*productFamilyTable)[productFamily] = gpu;
    }
};

// Main accessor to get a Gpu
const Gpu *getGpu(PRODUCT_FAMILY productFamily);

// Main accessor to get a CommandStreamerHelper
CommandStreamerHelper &getCommandStreamerHelper(PRODUCT_FAMILY productFamily, uint32_t device, EngineType engine);

} // namespace aub_stream
