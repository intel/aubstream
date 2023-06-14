/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"

namespace aub_stream {
constexpr uint32_t mmioDeviceOffset = 16 * MB;

template <typename Helper>
struct CommandStreamerHelperXeHpCore : public Helper {
    using Helper::Helper;

    void submitContext(AubStream &stream, std::array<MiContextDescriptorReg, 8> &contextDescriptor) const override {
        for (uint32_t i = 0; i < 8; i++) {
            stream.writeMMIO(mmioEngine + 0x2510 + (i * 8), contextDescriptor[i].ulData[0]);
            stream.writeMMIO(mmioEngine + 0x2514 + (i * 8), contextDescriptor[i].ulData[1]);
        }

        // Load our new exec list
        stream.writeMMIO(mmioEngine + 0x2550, 1);
    }

    const uint32_t getPollForCompletionMask() const override { return 0x00008000; }

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuXeHpCore : public Gpu {
    static const uint32_t numSupportedDevices = 4;

    CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const override;

    const std::vector<EngineType> getSupportedEngines() const override;

    const MMIOList getGlobalMMIO() const override;

    void initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const override;

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override;

    void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override;

    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const override;

    uint64_t getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize, uint64_t stolenMemoryBaseAddress) const override;

    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;

    void initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress) const;

    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const override;
};

} // namespace aub_stream
