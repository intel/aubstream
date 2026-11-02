/*
 * Copyright (C) 2025-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/tbx_shm_stream.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"

namespace aub_stream {

template <typename Helper>
struct CommandStreamerHelperXeCore : public Helper {
    using Helper::Helper;

    void setPriority(MiContextDescriptorReg &contextDescriptor, uint32_t priority) const override {

        if (priority != 0) {
            contextDescriptor.sData.FunctionType = priority & 0x3;
        }
    }

    void submitContext(AubStream &stream, std::vector<MiContextDescriptorReg> &contextDescriptor) const override {
        assert(contextDescriptor.size() <= 8);

        for (uint32_t i = 0; i < 8; i++) {
            stream.writeMMIO(mmioEngine + 0x2510 + (i * 8), contextDescriptor[i].ulData[0]);
            stream.writeMMIO(mmioEngine + 0x2514 + (i * 8), contextDescriptor[i].ulData[1]);
        }

        // Load our new exec list
        stream.writeMMIO(mmioEngine + 0x2550, 1);
    }

    uint32_t getPollForCompletionMask() const override { return 0x00008000; }

    // Contexts are never removed from a group, so valid descriptors are at the front; stop at the first invalid one.
    template <typename Descriptor>
    static size_t getElspSubmitCount(const std::vector<Descriptor> &contextDescriptor) {
        size_t count = 1;
        while (count < contextDescriptor.size() && contextDescriptor[count].sData.Valid) {
            count++;
        }
        return count;
    }

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuXeCore : public Gpu {
    void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override;

    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override;

    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;

    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const override;

    void initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress) const;

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override;
};

} // namespace aub_stream
