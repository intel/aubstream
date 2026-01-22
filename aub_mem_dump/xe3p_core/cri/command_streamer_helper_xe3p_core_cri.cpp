/*
 * Copyright (C) 2025-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe3p_core/command_streamer_helper_xe3p_core.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "aubstream/product_family.h"

namespace aub_stream {
struct GpuCri : public GpuXe3pCore {
    GpuCri() {
        productFamily = ProductFamily::Cri;
        gfxCoreFamily = CoreFamily::Xe3pCore;
        productAbbreviation = "cri";
        deviceId = 0;
        deviceCount = GpuXe3pCore::numSupportedDevices;
    }

    const MMIOList getGlobalMMIOPlatformSpecific() const override {
        const MMIOList globalMMIOPlatformSpecific = {
            // bits: 8: IG_PAT, 7-6: L3_CLOS, 5-4: L3_CACHE_POLICY, 3-2:L4_CACHE_POLICY
            MMIOPair(0x00004000, 0b0000'0000'1100), // Defer to PAT
            MMIOPair(0x00004004, 0b0001'0011'1100), // UC
            MMIOPair(0x00004008, 0b0001'0000'1100), // L2
            MMIOPair(0x0000400C, 0b0001'0011'0000), // L3
            MMIOPair(0x00004010, 0b0001'0000'0000), // L2 + L3

            // bits: 10: NO_PROMOTE, 9: COMP_EN, 7-6: L3_CLOS, 5-4: L3_CACHE_POLICY, 3-2:L4_CACHE_POLICY, 1-0: COH_MODE
            MMIOPair(getPatIndexMmioAddr(0), 0b0000'0000'0000),  // L2 + L3
            MMIOPair(getPatIndexMmioAddr(1), 0b0000'0000'0010),  // L2 + L3 (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(2), 0b0000'0000'0011),  // L2 + L3 (2-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(3), 0b0000'0011'1100),  // UC
            MMIOPair(getPatIndexMmioAddr(4), 0b0000'0011'1110),  // UC (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(5), 0b0000'0011'0000),  // L3
            MMIOPair(getPatIndexMmioAddr(6), 0b0000'0011'0010),  // L3 (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(7), 0b0000'0011'0011),  // L3 (2-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(8), 0b0000'0000'1100),  // L2
            MMIOPair(getPatIndexMmioAddr(9), 0b0000'0000'1110),  // L2 (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(10), 0b0000'0000'1111), // L2 (2-Way Coherent)
            // PatIndex 11..22 Reserved
            MMIOPair(getPatIndexMmioAddr(23), 0b0000'0100'0000), // CLOS1 L2 + L3
            MMIOPair(getPatIndexMmioAddr(24), 0b0000'0100'0010), // CLOS1 L2 + L3 (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(25), 0b0000'0100'0011), // CLOS1 L2 + L3 (2-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(26), 0b0000'1000'0000), // CLOS2 L2 + L3
            MMIOPair(getPatIndexMmioAddr(27), 0b0000'1000'0010), // CLOS2 L2 + L3 (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(28), 0b0000'1000'0011), // CLOS2 L2 + L3 (2-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(29), 0b0000'1100'0000), // CLOS3 L2 + L3
            MMIOPair(getPatIndexMmioAddr(30), 0b0000'1100'0010), // CLOS3 L2 + L3 (1-Way Coherent)
            MMIOPair(getPatIndexMmioAddr(31), 0b0000'1100'0011), // CLOS3 L2 + L3 (2-Way Coherent)

            MMIOPair(0x000047FC, 0b0000'0000'0011), // PAT_ATS: PCIe ATS/PASID
            MMIOPair(0x00004820, 0b0000'0000'0000), // PTA_MODE: PPGTT
        };
        return globalMMIOPlatformSpecific;
    };

    const std::vector<EngineType> getSupportedEngines() const override {
        static constexpr std::array<EngineType, 15> engines = {{ENGINE_BCS, ENGINE_VCS, ENGINE_VECS,
                                                                ENGINE_CCS, ENGINE_CCS1, ENGINE_CCS2, ENGINE_CCS3,
                                                                ENGINE_BCS1, ENGINE_BCS2, ENGINE_BCS3, ENGINE_BCS4, ENGINE_BCS5,
                                                                ENGINE_BCS6, ENGINE_BCS7, ENGINE_BCS8}};
        return std::vector<EngineType>(engines.begin(), engines.end());
    }

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override {
        assert(MEMORY_BANK_SYSTEM == 0);
        auto supportedBanks = MEMORY_BANK_SYSTEM | MEMORY_BANK_0 | MEMORY_BANK_1 | MEMORY_BANK_2 | MEMORY_BANK_3;
        auto unsupportedBanks = MEMORY_BANKS_ALL ^ supportedBanks;

        if (unsupportedBanks & memoryBanks) {
            return false;
        }

        if (memoryBanks & supportedBanks) {
            return alignment == 65536 || alignment == Page2MB::pageSize2MB;
        }

        return alignment == 4096 || alignment == 65536;
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Cri>() {
    return std::make_unique<GpuCri>;
}

} // namespace aub_stream
