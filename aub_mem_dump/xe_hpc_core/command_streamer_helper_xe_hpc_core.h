/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/xe_hp_core/command_streamer_helper_xe_hp_core.h"

namespace aub_stream {

template <typename Helper>
struct CommandStreamerHelperXeHpcCore : public CommandStreamerHelperXeHpCore<Helper> {
    using CommandStreamerHelperXeHpCore<Helper>::CommandStreamerHelperXeHpCore;

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuXeHpcCore : public GpuXeHpCore {
    GpuXeHpcCore();
    static const uint32_t numSupportedDevices = 4;
    static constexpr uint64_t patIndexBit0 = toBitValue(3);
    static constexpr uint64_t patIndexBit1 = toBitValue(4);
    static constexpr uint64_t patIndex0 = 0;                           // 0b0000
    static constexpr uint64_t patIndex3 = patIndexBit0 | patIndexBit1; // 0b0011

    const std::vector<EngineType> getSupportedEngines() const override;

    static constexpr uint32_t getPatIndexMmioAddr(uint32_t index);

    const MMIOList getGlobalMMIO() const override;

    uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const override;

    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;
};

} // namespace aub_stream
