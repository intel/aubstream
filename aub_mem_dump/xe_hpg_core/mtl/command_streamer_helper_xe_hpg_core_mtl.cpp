/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hpg_core/command_streamer_helper_xe_hpg_core.h"

#include "aubstream/product_family.h"
namespace aub_stream {

struct GpuMtl : public GpuXeHpgCore {
    GpuMtl() {
        productFamily = ProductFamily::Mtl;
        gfxCoreFamily = CoreFamily::XeHpgCore;
        productAbbreviation = "mtl";
        deviceId = 42;
        deviceCount = GpuXeHpgCore::numSupportedDevices;
    }

    uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const override {
        auto bits = toBitValue(PpgttEntryBits::atomicEnableBit);

        bits |= allocationParams.uncached ? patIndex2 : patIndex0;

        return bits;
    }

    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const override {}

    const MMIOList getGlobalMMIOPlatformSpecific() const override {
        const MMIOList globalMMIOPlatformSpecific = {

            // bits: 3-2: L4_CACHE_POLICY, 1-0: COH_MODE
            MMIOPair(getPatIndexMmioAddr(0), 0b0000), // PAT_INDEX 0: WB, Non-Coherent
            MMIOPair(getPatIndexMmioAddr(1), 0b0100), // PAT_INDEX 1: WT, Non-Coherent
            MMIOPair(getPatIndexMmioAddr(2), 0b1100), // PAT_INDEX 2: UC, Non-Coherent
            MMIOPair(getPatIndexMmioAddr(3), 0b0010), // PAT_INDEX 3: WB, 1-Way Coherent
            MMIOPair(getPatIndexMmioAddr(4), 0b0011), // PAT_INDEX 4: WB, 2-Way Coherent
        };
        return globalMMIOPlatformSpecific;
    };

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
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Mtl>() {
    return std::make_unique<GpuMtl>;
}
} // namespace aub_stream
