/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/options.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/alloc_tools.h"
#include <memory>

namespace aub_stream {

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

void Gpu::initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const {
    const auto &globalMMIO = getGlobalMMIO();
    for (const auto &mmioPair : globalMMIO) {
        stream.writeMMIO(mmioPair.first, mmioPair.second);
    }

    const auto &globalMMIOPlatformSpecific = getGlobalMMIOPlatformSpecific();
    for (const auto &mmioPair : globalMMIOPlatformSpecific) {
        stream.writeMMIO(mmioPair.first, mmioPair.second);
    }

    // Add injected MMIO
    for (const auto &mmioPair : MMIOListInjected) {
        stream.writeMMIO(mmioPair.first, mmioPair.second);
    }
}

StolenMemoryInHeap::StolenMemoryInHeap(uint32_t deviceCount, uint64_t memoryBankSize) {
    const uint64_t flatCcsSize = memoryBankSize / 256;
    for (uint32_t d = 0; d < deviceCount; ++d) {
        auto p = std::unique_ptr<uint8_t, decltype(&aligned_free)>(reinterpret_cast<uint8_t *>(aligned_alloc(static_cast<size_t>(flatCcsSize + 8 * MB + 1 * MB), static_cast<size_t>(MB))), &aligned_free);
        localStolenStorage.push_back(std::move(p));
    }
}

uint64_t StolenMemoryInHeap::getBaseAddress(uint32_t device) const {
    return reinterpret_cast<uint64_t>(localStolenStorage[device].get());
}

StolenMemoryInStaticStorage::StolenMemoryInStaticStorage(uint64_t memoryBankSize) : staticMemoryBankSize(memoryBankSize) {
}

uint64_t StolenMemoryInStaticStorage::getBaseAddress(uint32_t device) const {
    const uint64_t flatCcsSize = staticMemoryBankSize / 256;
    const uint64_t ggttSize = 8 * MB;
    const uint64_t wopcmSize = 1 * MB;
    uint64_t baseAddr = staticMemoryBankSize * (device + 1);
    baseAddr -= flatCcsSize;
    baseAddr -= ggttSize;
    baseAddr -= wopcmSize;
    return baseAddr;
}

std::unique_ptr<StolenMemory> StolenMemory::CreateStolenMemory(bool inHeap, uint32_t deviceCount, uint64_t memoryBankSize) {
    if (inHeap) {
        return std::make_unique<StolenMemoryInHeap>(deviceCount, memoryBankSize);
    } else {
        return std::make_unique<StolenMemoryInStaticStorage>(memoryBankSize);
    }
}

} // namespace aub_stream
