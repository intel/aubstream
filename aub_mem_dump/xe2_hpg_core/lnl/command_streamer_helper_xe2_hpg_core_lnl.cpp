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
struct GpuLnl : public GpuXe2HpgCore {

    GpuLnl() {
        productFamily = ProductFamily::Lnl;
        gfxCoreFamily = CoreFamily::Xe2HpgCore;
        productAbbreviation = "lnl";
        deviceId = 45;
        deviceCount = GpuXe2HpgCore::numSupportedDevices;
    }

    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const override {
        assert(deviceCount > 0u);
        assert(deviceCount <= this->deviceCount);
        assert(GpuLnl::numSupportedDevices == this->deviceCount);

        const uint32_t mmioDevice[1] = {0};
        const uint32_t gsmBase = 0x108100;

        uint64_t gttBase = getGGTTBaseAddress(0, memoryBankSize, stolenMemory.getBaseAddress(0));
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
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Lnl>() {
    return std::make_unique<GpuLnl>;
}
} // namespace aub_stream
