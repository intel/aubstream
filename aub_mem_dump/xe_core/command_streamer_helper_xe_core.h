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
#include <mutex>
#include <span>
#include <vector>

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

struct TlbInvalidationDescriptor {
    static constexpr uint32_t granularityAllMappingsWithinAllAsidAndVf = 0;
    static constexpr uint32_t invalidationModeHeavy = 0;
    static constexpr uint32_t descriptorTypeTlbInvalidation = 0;

    // Dword 0
    uint32_t Valid : 1;
    uint32_t Granularity : 2;
    uint32_t AddressMask : 6;
    uint32_t InvalidationMode : 1;
    uint32_t DescriptorType : 2;
    uint32_t Asid : 20;
    // Dword 1
    uint32_t Vfid : 6;
    uint32_t AddressLow : 26;
};
static_assert(sizeof(TlbInvalidationDescriptor) == sizeof(uint32_t) * 2);

struct GpuXeCore : public Gpu {
    void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override;

    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override;

    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;

    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const override;

    void initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress) const;

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override;

    bool invalidateAllTLBs(AubStream &stream, uint32_t devicesCount) const override;

  protected:
    virtual std::vector<uint32_t> getInvalidateAllTlbsDescriptor() const;
    virtual uint32_t getTlbInvalidationAckStatusRegister() const { return 0x0CF7C; }
    bool writeTlbInvalidationDescriptorAndWaitForAck(AubStream &stream, uint32_t mmioDevice, std::span<const uint32_t> descriptorDwords, uint32_t ackStatusRegister) const;

    mutable std::mutex tlbInvalidationMutex;
};

} // namespace aub_stream
