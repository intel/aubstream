/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hpg_core/command_streamer_helper_xe_hpg_core.h"

#include "aubstream/product_family.h"
namespace aub_stream {

struct GpuDg2 : public GpuXeHpgCore {
    GpuDg2() {
        productFamily = ProductFamily::Dg2;
        gfxCoreFamily = CoreFamily::XeHpgCore;
        productAbbreviation = "dg2";
        deviceId = 0x24;
        deviceCount = GpuXeHpgCore::numSupportedDevices;
    }

    const std::vector<EngineType> getSupportedEngines() const override {
        static constexpr std::array<EngineType, 7> engines = {{ENGINE_RCS, ENGINE_BCS,
                                                               ENGINE_CCS, ENGINE_CCS1, ENGINE_CCS2, ENGINE_CCS3}};
        return std::vector<EngineType>(engines.begin(), engines.end());
    }

    uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const override {
        return toBitValue(PpgttEntryBits::atomicEnableBit);
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Dg2>() {
    return std::make_unique<GpuDg2>;
}
} // namespace aub_stream
