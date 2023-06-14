/*
 * Copyright (C) 2022-2023 Intel Corporation
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

    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const override {}

    const MMIOList getGlobalMMIOPlatformSpecific() const override {
        const MMIOList globalMMIOPlatformSpecific = {
            MMIOPair(getPatIndexMmioAddr(0), 0x0), // PAT_INDEX 0: WB, Non-Coherent
            MMIOPair(getPatIndexMmioAddr(1), 0x4), // PAT_INDEX 1: WT, Non-Coherent
            MMIOPair(getPatIndexMmioAddr(2), 0xC), // PAT_INDEX 2: UC, Non-Coherent
            MMIOPair(getPatIndexMmioAddr(3), 0x2), // PAT_INDEX 3: WB, 1-Way Coherent
            MMIOPair(getPatIndexMmioAddr(4), 0x3), // PAT_INDEX 4: WB, 2-Way Coherent
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
const Gpu *enableGpu<ProductFamily::Mtl>() {
    static const GpuMtl mtl;
    return &mtl;
}

} // namespace aub_stream
