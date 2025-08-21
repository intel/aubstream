/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/xe_core/command_streamer_helper_xe_core.h"

namespace aub_stream {

template <typename Helper>
struct CommandStreamerHelperXeHpcCore : public CommandStreamerHelperXeCore<Helper> {
    using CommandStreamerHelperXeCore<Helper>::CommandStreamerHelperXeCore;

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuXeHpcCore : public GpuXeCore {
    GpuXeHpcCore();
    static const uint32_t numSupportedDevices = 4;
    static constexpr uint64_t patIndexBit0 = toBitValue(3);
    static constexpr uint64_t patIndexBit1 = toBitValue(4);
    static constexpr uint64_t patIndex0 = 0;                           // 0b0000
    static constexpr uint64_t patIndex3 = patIndexBit0 | patIndexBit1; // 0b0011

    CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const override;
    const std::vector<EngineType> getSupportedEngines() const override;

    static constexpr uint32_t getPatIndexMmioAddr(uint32_t index);

    const MMIOList getGlobalMMIO() const override;

    uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const override;

    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;
    std::unique_ptr<CommandStreamerHelper> commandStreamerHelperTable[numSupportedDevices][EngineType::NUM_ENGINES];
};

} // namespace aub_stream
