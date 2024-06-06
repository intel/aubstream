/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/xe_hp_core/command_streamer_helper_xe_hp_core.h"

namespace aub_stream {

template <typename Helper>
struct CommandStreamerHelperXe2HpgCore : public CommandStreamerHelperXeHpCore<Helper> {
    using CommandStreamerHelperXeHpCore<Helper>::CommandStreamerHelperXeHpCore;

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuXe2HpgCore : public GpuXeHpCore {
    GpuXe2HpgCore();
    static constexpr uint32_t numSupportedDevices = 1;
    static constexpr uint64_t patIndexBit0 = toBitValue(3);
    static constexpr uint64_t patIndexBit1 = toBitValue(4);
    static constexpr uint64_t patIndexBit2 = toBitValue(7);
    static constexpr uint64_t patIndexBit3 = toBitValue(62);
    static constexpr uint64_t patIndex0 = 0;                            // 0b0000
    static constexpr uint64_t patIndex2 = patIndexBit1;                 // 0b0010
    static constexpr uint64_t patIndex8 = patIndexBit3;                 // 0b1000
    static constexpr uint64_t patIndex9 = patIndexBit3 | patIndexBit0;  // 0b1001
    static constexpr uint64_t patIndex12 = patIndexBit2 | patIndexBit3; // 0b1100

    const std::vector<EngineType> getSupportedEngines() const override;

    static constexpr uint32_t getPatIndexMmioAddr(uint32_t index);
    const MMIOList getGlobalMMIO() const override;

    uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const override;
    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;
    bool isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const override;
    uint32_t sizeToGMS(uint64_t dataStolenMemorySize) const;

    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const override;
    void initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress, uint64_t size) const;
    void initializeTileRangeMmio(AubStream &stream, uint32_t deviceIndex, uint64_t lmemBaseAddress, uint64_t lmemSize) const;

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override;
};
} // namespace aub_stream
