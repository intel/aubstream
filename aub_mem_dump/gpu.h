/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_stream.h"
#include "aubstream/engine_node.h"
#include "aubstream/allocation_params.h"

#include "aubstream/aubstream.h"
#include "gfx_core_family.h"

#include <array>
#include <vector>
#include <string>

namespace aub_stream {

struct AubStream;
struct CommandStreamerHelper;
struct GGTT;
struct PageTable;
struct PhysicalAddressAllocator;
enum class ProductFamily : uint32_t;

struct StolenMemory {
    virtual uint64_t getBaseAddress(uint32_t device) const = 0;
    static std::unique_ptr<StolenMemory> CreateStolenMemory(bool inHeap, uint32_t deviceCount, uint64_t memoryBankSize);
    virtual ~StolenMemory() = default;
};

struct StolenMemoryInHeap : public StolenMemory {
    StolenMemoryInHeap(uint32_t deviceCount, uint64_t memoryBankSize);
    uint64_t getBaseAddress(uint32_t device) const override;

  protected:
    std::vector<std::unique_ptr<uint8_t, decltype(&aligned_free)>> localStolenStorage;
};

struct StolenMemoryInStaticStorage : public StolenMemory {
    StolenMemoryInStaticStorage(uint64_t memoryBankSize);
    uint64_t getBaseAddress(uint32_t device) const override;

  protected:
    uint64_t staticMemoryBankSize;
};

struct GpuDescriptor {
    ProductFamily productFamily{};
    CoreFamily gfxCoreFamily{};
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
    virtual void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const = 0;

    virtual bool requireLocalMemoryForPageTables() const { return false; }
    virtual const MMIOList getGlobalMMIOPlatformSpecific() const { return {}; }

    virtual void initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const;
    virtual void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {};
    bool isEngineSupported(uint32_t engine) const;

    virtual uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const { return 0; }

    virtual GGTT *allocateGGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress) const;
    virtual PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const;

    virtual uint64_t getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize, uint64_t stolenMemoryBaseAddress) const = 0;
};

} // namespace aub_stream
