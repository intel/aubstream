/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"
#include "aub_services.h"
#include "aubstream/product_family.h"

namespace aub_stream {

struct GpuKbl : public GpuGen9 {
    GpuKbl() {
        productFamily = ProductFamily::Kbl;
        gfxCoreFamily = CoreFamily::Gen9;
        productAbbreviation = "kbl";
        deviceId = CmdServicesMemTraceVersion::DeviceValues::Kbl;
        deviceCount = 1;
    }
    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override {
        //Kabylake page walks seems to be broken when using 64KB pages , so we are limiting it to 4K pages
        return memoryBanks == MEMORY_BANK_SYSTEM && (alignment == 4096);
    }
};
template <>
const Gpu *enableGpu<ProductFamily::Kbl>() {
    static const GpuKbl kbl;
    return &kbl;
}

} // namespace aub_stream
