/*
 * Copyright (C) 2024-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe3_core/command_streamer_helper_xe3_core.h"
#include "aubstream/product_family.h"
#include "aub_mem_dump/page_table_pml5.h"

namespace aub_stream {

struct GpuPtl : public GpuXe3Core {
    static constexpr uint32_t numSupportedDevices = 1;

    GpuPtl() {
        productFamily = ProductFamily::Ptl;
        gfxCoreFamily = CoreFamily::Xe3Core;
        productAbbreviation = "ptl";
        deviceId = 51;
        deviceCount = GpuPtl::numSupportedDevices;
    }

    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override {
        assert(deviceCount > 0u);
        assert(deviceCount <= this->deviceCount);
        assert(GpuPtl::numSupportedDevices == this->deviceCount);

        const uint32_t mmioDevice[1] = {0};
        const uint32_t gsmBase = 0x108100;

        uint64_t gttBase = getGSMBaseAddress(0);
        stream.writeMMIO(mmioDevice[0] + gsmBase + 4, static_cast<uint32_t>(gttBase >> 32));
        stream.writeMMIO(mmioDevice[0] + gsmBase + 0, static_cast<uint32_t>(gttBase & 0xFFF00000));
    }

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override {
        assert(MEMORY_BANK_SYSTEM == 0);
        auto supportedBanks = MEMORY_BANK_SYSTEM;
        auto unsupportedBanks = MEMORY_BANKS_ALL ^ supportedBanks;

        if (unsupportedBanks & memoryBanks) {
            return false;
        }

        // MEMORY_BANK_SYSTEM
        return alignment == 4096 || alignment == 65536;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Ptl>() {
    return std::make_unique<GpuPtl>;
}
} // namespace aub_stream
