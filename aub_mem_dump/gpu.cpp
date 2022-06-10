/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/options.h"
#include "aub_mem_dump/page_table.h"

#include "aub_mem_dump/gpu.h"

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

} // namespace aub_stream
