/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe2_hpg_core/command_streamer_helper_xe2_hpg_core.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "aubstream/product_family.h"

namespace aub_stream {

struct GpuBmg : public GpuXe2HpgCore {
    GpuBmg() {
        productFamily = ProductFamily::Bmg;
        gfxCoreFamily = CoreFamily::Xe2HpgCore;
        productAbbreviation = "bmg";
        deviceId = 43;
        deviceCount = GpuXe2HpgCore::numSupportedDevices;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Bmg>() {
    return std::make_unique<GpuBmg>;
}
} // namespace aub_stream
