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
#include <functional>
#include <memory>

namespace aub_stream {

struct CommandStreamerHelper;
struct Gpu;
enum class ProductFamily : uint32_t;

// Table of HW family specific Gpus
extern std::unique_ptr<std::map<ProductFamily, std::function<std::unique_ptr<Gpu>()>>> productFamilyTable;

// Helper method to access a productFamily Gpu
template <ProductFamily>
std::function<std::unique_ptr<Gpu>()> enableGpu();

// Helper to register product families
template <ProductFamily productFamily>
struct RegisterFamily {
    RegisterFamily() {
        auto createGpuFunc = enableGpu<productFamily>();
        if (!productFamilyTable) {
            productFamilyTable = std::make_unique<std::map<ProductFamily, std::function<std::unique_ptr<Gpu>()>>>();
        }
        (*productFamilyTable.get())[productFamily] = std::move(createGpuFunc);
    }
};

// Main accessor to get a Gpu
std::function<std::unique_ptr<Gpu>()> getGpu(ProductFamily productFamily);
} // namespace aub_stream
