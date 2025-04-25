/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/memory_banks.h"
#include "aubstream/engine_node.h"
#include "aubstream/allocation_params.h"

#include "aubstream/aubstream.h"
#include "gfx_core_family.h"

#include <array>
#include <vector>
#include <string>
#include <memory>

namespace aub_stream {

struct AubStream;
struct CommandStreamerHelper;
struct GGTT;
struct PageTable;
struct PhysicalAddressAllocator;
enum class ProductFamily : uint32_t;

constexpr uint32_t mmioDeviceOffset = 16 * MB;

struct StolenMemory {
    StolenMemory(uint64_t dataStolenMemorySize) : dsmSize(dataStolenMemorySize) {}
    virtual uint64_t getBaseAddress(uint32_t device) const = 0;
    static std::unique_ptr<StolenMemory> CreateStolenMemory(bool inHeap, uint32_t deviceCount, uint64_t memoryBankSize, uint64_t dataStolenMemorySize);
    virtual ~StolenMemory() = default;

    uint64_t dsmSize;

    // Defaults
    static const uint64_t ggttSize = 8 * 1024 * 1024;
};

struct StolenMemoryInHeap : public StolenMemory {
    StolenMemoryInHeap(uint32_t deviceCount, uint64_t memoryBankSize, uint64_t dataStolenMemorySize);
    uint64_t getBaseAddress(uint32_t device) const override;

  protected:
    std::vector<std::unique_ptr<uint8_t, decltype(&aligned_free)>> localStolenStorage;
};

struct StolenMemoryInStaticStorage : public StolenMemory {
    StolenMemoryInStaticStorage(uint64_t memoryBankSize, uint64_t dataStolenMemorySize);
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
    virtual ~GpuDescriptor() = default;
};

struct Gpu : public GpuDescriptor {
    Gpu();
    ~Gpu() override;
    static constexpr uint32_t numSupportedDevices = 4;
    CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const;
    virtual const MMIOList getGlobalMMIO() const = 0;
    virtual bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const = 0;
    virtual const std::vector<EngineType> getSupportedEngines() const = 0;
    virtual void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const = 0;
    virtual void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const = 0;

    virtual bool requireLocalMemoryForPageTables() const { return false; }
    virtual const MMIOList getGlobalMMIOPlatformSpecific() const { return {}; }

    virtual void initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const;
    virtual void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {};
    virtual void injectMMIOs(AubStream &stream, uint32_t devicesCount) const;
    bool isEngineSupported(uint32_t engine) const;

    virtual bool isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const;

    virtual uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const { return 0; }

    virtual GGTT *allocateGGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress) const;
    virtual PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const;

    virtual uint64_t getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize, uint64_t stolenMemoryBaseAddress) const = 0;

    virtual uint32_t getContextGroupCount() const {
        return 8;
    }
    std::unique_ptr<CommandStreamerHelper> commandStreamerHelperTable[Gpu::numSupportedDevices][EngineType::NUM_ENGINES];
};

} // namespace aub_stream
