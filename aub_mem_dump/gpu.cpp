/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/options.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/alloc_tools.h"
#include "aub_mem_dump/align_helpers.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include <memory>

namespace aub_stream {

Gpu::Gpu() = default;
Gpu::~Gpu() = default;

bool Gpu::isEngineSupported(uint32_t engine) const {
    auto &supportedEngines = getSupportedEngines();
    return (std::find(supportedEngines.begin(), supportedEngines.end(), engine)) != supportedEngines.end();
};

GGTT *Gpu::allocateGGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress) const {
    return new GGTT(*this, physicalAddressAllocator, memoryBank, gttBaseAddress);
}

PageTable *Gpu::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    return new LegacyPML4(*this, physicalAddressAllocator, memoryBank);
}

bool Gpu::isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const {
    const uint64_t mask1MB = (uint64_t(1) << 20) - 1;
    // Stolen memory must be 1MB aligned
    if ((dataStolenMemorySize & mask1MB) != 0) {
        return false;
    }
    if (dataStolenMemorySize > 2ull * 1024ull * 1024ull * 1024ull) {
        return false;
    }
    return true;
}

void Gpu::initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const {
    const auto &globalMMIO = getGlobalMMIO();
    for (const auto &mmioPair : globalMMIO) {
        stream.writeMMIO(mmioPair.first, mmioPair.second);
    }

    const auto &globalMMIOPlatformSpecific = getGlobalMMIOPlatformSpecific();
    for (const auto &mmioPair : globalMMIOPlatformSpecific) {
        stream.writeMMIO(mmioPair.first, mmioPair.second);
    }
}

CommandStreamerHelper &Gpu::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    auto &csh = commandStreamerHelperTable[device][engineType];
    csh->gpu = this;
    return *csh;
}

void Gpu::injectMMIOs(AubStream &stream, uint32_t devicesCount) const {
    uint32_t mmioDevice = 0;

    for (uint32_t device = 0; device < devicesCount; device++) {
        // Add injected MMIO
        for (const auto &mmioPair : MMIOListInjected) {
            stream.writeMMIO(mmioDevice + mmioPair.first, mmioPair.second);
        }

        mmioDevice += mmioDeviceOffset;
    }
}

StolenMemoryInHeap::StolenMemoryInHeap(uint32_t deviceCount, uint64_t memoryBankSize, uint64_t dataStolenMemorySize) : StolenMemory(dataStolenMemorySize) {
    // Some platforms require to allocate 1/512th portion of mem and others
    // require to allocate 1/256th, so allocating 1/256th will cover all needs
    const uint64_t flatCcsSize = memoryBankSize / 256;
    // Flat CCS buffer size must be 1MB aligned to make sure that there is enough space to make GTT base address to be also aligned to 1MB
    const uint64_t flatCcsSizeAligned = alignUp(flatCcsSize, 20);
    for (uint32_t d = 0; d < deviceCount; ++d) {
        auto p = std::unique_ptr<uint8_t, decltype(&aligned_free)>(reinterpret_cast<uint8_t *>(aligned_alloc(static_cast<size_t>(flatCcsSizeAligned + ggttSize + dataStolenMemorySize), static_cast<size_t>(MB))), &aligned_free);
        localStolenStorage.push_back(std::move(p));
    }
}

uint64_t StolenMemoryInHeap::getBaseAddress(uint32_t device) const {
    return reinterpret_cast<uint64_t>(localStolenStorage[device].get());
}

StolenMemoryInStaticStorage::StolenMemoryInStaticStorage(uint64_t memoryBankSize, uint64_t dataStolenMemorySize) : StolenMemory(dataStolenMemorySize),
                                                                                                                   staticMemoryBankSize(memoryBankSize) {
}

uint64_t StolenMemoryInStaticStorage::getBaseAddress(uint32_t device) const {
    // Some platforms require to allocate 1/512th portion of mem and others
    // require to allocate 1/256th, so allocating 1/256th will cover all needs
    const uint64_t flatCcsSize = staticMemoryBankSize / 256;
    // Flat CCS buffer size must be 1MB aligned to make sure that there is enough space to make GTT base address to be also aligned to 1MB
    const uint64_t flatCcsSizeAligned = alignUp(flatCcsSize, 20);
    uint64_t baseAddr = staticMemoryBankSize * (device + 1);
    baseAddr -= flatCcsSizeAligned;
    baseAddr -= ggttSize;
    baseAddr -= dsmSize;
    // Base address must be 1MB aligned to make GTT base address also 1MB aligned
    return alignDown(baseAddr, 20);
}

std::unique_ptr<StolenMemory> StolenMemory::CreateStolenMemory(bool inHeap, uint32_t deviceCount, uint64_t memoryBankSize, uint64_t dataStolenMemorySize) {
    const uint64_t flatCcsSize = memoryBankSize / 256;
    const uint64_t flatCcsSizeAligned = alignUp(flatCcsSize, 20);

    // Make sure that memory is enough
    if (memoryBankSize < dataStolenMemorySize + ggttSize + flatCcsSizeAligned) {
        return nullptr;
    }

    if (inHeap) {
        return std::make_unique<StolenMemoryInHeap>(deviceCount, memoryBankSize, dataStolenMemorySize);
    } else {
        return std::make_unique<StolenMemoryInStaticStorage>(memoryBankSize, dataStolenMemorySize);
    }
}

} // namespace aub_stream
