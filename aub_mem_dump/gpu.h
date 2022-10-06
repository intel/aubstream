/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_stream.h"
#include "gfx_core_family.h"
#include "aubstream/headers/engine_node.h"
#include "aubstream/headers/allocation_params.h"

#include "product_family.h"

#include <array>
#include <vector>

namespace aub_stream {

struct CommandStreamerHelper;
struct PageTable;

struct GpuDescriptor {
    PRODUCT_FAMILY productFamily{};
    GFXCORE_FAMILY gfxCoreFamily{};
    std::string productAbbreviation;
    uint32_t deviceId{};
    uint32_t deviceCount{};
};

struct Gpu : public GpuDescriptor {
    virtual CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const = 0;
    virtual const MMIOList getGlobalMMIO() const = 0;
    virtual bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const = 0;
    virtual const std::vector<EngineType> getSupportedEngines() const = 0;
    virtual void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const = 0;
    virtual void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const = 0;

    virtual bool requireLocalMemoryForPageTables() const { return false; }
    virtual const MMIOList getGlobalMMIOPlatformSpecific() const { return {}; }

    virtual void initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const;
    virtual void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const {};
    bool isEngineSupported(uint32_t engine) const;

    virtual uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const { return 0; }

    virtual GGTT *allocateGGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress) const;
    virtual PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const;

    virtual uint64_t getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize) const = 0;
};

} // namespace aub_stream
