/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/align_helpers.h"
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
    StolenMemory(uint64_t size) : size(size) {}
    virtual uint64_t getBaseAddress(uint32_t device) const = 0;
    static std::unique_ptr<StolenMemory> CreateStolenMemory(bool inHeap, uint32_t deviceCount, uint64_t memoryBankSize, uint64_t size);
    virtual ~StolenMemory() = default;

    uint64_t size;
};

struct StolenMemoryInHeap : public StolenMemory {
    StolenMemoryInHeap(uint32_t deviceCount, uint64_t size);
    uint64_t getBaseAddress(uint32_t device) const override;

  protected:
    std::vector<std::unique_ptr<uint8_t, decltype(&aligned_free)>> localStolenStorage;
};

struct StolenMemoryInStaticStorage : public StolenMemory {
    StolenMemoryInStaticStorage(uint64_t memoryBankSize, uint64_t size);
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
    virtual void injectMMIOs(AubStream &stream, uint32_t devicesCount) const;
    bool isEngineSupported(uint32_t engine) const;

    virtual bool isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const;

    virtual uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const { return 0; }

    virtual GGTT *allocateGGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress) const;
    virtual PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const;

    virtual uint64_t getWOPCMSize() const { return 8 * MB; };
    virtual uint64_t getDSMSize() const {
        if (dsmSizeOverride) {
            return dsmSizeOverride;
        }
        return getDSMDefaultSize();
    };
    virtual uint64_t getDSMDefaultSize() const { return getWOPCMSize(); };
    virtual uint64_t getGSMSize() const { return 8 * MB; };
    virtual uint64_t getFlatCCSSize(uint64_t memoryBankSize) const {
        // Some platforms require to allocate 1/512th portion of mem and others
        // require to allocate 1/256th, so allocating 1/256th will cover all needs
        // Flat CCS buffer size must be 1MB aligned to make sure that there is enough space to make GTT base address to be also aligned to 1MB
        return alignUp(memoryBankSize / 256, 20);
    }
    virtual uint64_t getStolenMemorySize(uint64_t memoryBankSize) {
        return getFlatCCSSize(memoryBankSize) + getGSMSize() + getDSMSize();
    }

    void overrideDSMSize(uint64_t dsmSize) {
        dsmSizeOverride = dsmSize;
    }

    virtual uint64_t getWOPCMBaseAddress(uint32_t device) const {
        // Top of DSM
        return getDSMBaseAddress(device) + getDSMSize() - getWOPCMSize();
    }
    virtual uint64_t getDSMBaseAddress(uint32_t device) const {
        // Top of stolen memory
        assert(stolenMemory != nullptr);
        return stolenMemory->getBaseAddress(device) + stolenMemory->size - getDSMSize();
    };
    virtual uint64_t getGSMBaseAddress(uint32_t device) const {
        // Below DSM; Stores GGTT
        return getDSMBaseAddress(device) - getGSMSize();
    }

    virtual uint32_t getContextGroupCount() const {
        return 8;
    }

    std::unique_ptr<StolenMemory> stolenMemory;

  protected:
    uint64_t dsmSizeOverride = 0;
};

} // namespace aub_stream
