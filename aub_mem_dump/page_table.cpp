/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gpu.h"
#include "page_table.h"
#include <algorithm>
#include <cassert>

namespace aub_stream {

PageTable::PageTable(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, size_t size, unsigned int tableCount, uint32_t memoryBank)
    : gpu(gpu),
      allocator(physicalAddressAllocator),
      physicalAddress(0u),
      memoryBank(memoryBank) {
    // Allocate dedicate memory for table if requested
    if (size) {
        size = std::max(size, size_t(4096));
        physicalAddress = allocator->reservePhysicalMemory(memoryBank, size, size);
    }
}

uint64_t PageTable::getEntryValue() const {
    auto bits = toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    bits |= isLocalMemory() ? toBitValue(PpgttEntryBits::localMemoryBit) : 0;
    bits |= gpu.getPPGTTExtraEntryBits(additionalAllocParams);
    return getPhysicalAddress() | bits;
}

uint64_t Page2MB::getEntryValue() const {
    assert((getPhysicalAddress() & (Page2MB::pageSize2MB - 1)) == 0 && "Page2MB physical address must be 2MB aligned");
    auto bits = toBitValue(PpgttEntryBits::largePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    bits |= isLocalMemory() ? toBitValue(PpgttEntryBits::localMemoryBit) : 0;
    bits |= gpu.getPPGTTExtraEntryBits(additionalAllocParams);
    return getPhysicalAddress() | bits;
}

PageTable::~PageTable() {
    if (allocator) {
        allocator->freePhysicalMemory(memoryBank, physicalAddress);
    }
    for (auto &entry : table) {
        delete entry;
    }
}

GGTT::GGTT(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress)
    : PageTable(gpu, physicalAddressAllocator, 0u, 1u << 20, memoryBank),
      entryOffset(0),
      gfxAddressAllocator(0u),
      gttTableOffset(gttBaseAddress) {
    numAddressBits = 32;
    numLevels = 1;
    // must be system memory or only have 1 bit set.  No coloring for GGTT
    assert(0 == memoryBank || 0 == (memoryBank & (memoryBank - 1)));

    while (memoryBank && !(memoryBank & 1)) {
        entryOffset += 0x1000000;
        memoryBank >>= 1;
    }
}

} // namespace aub_stream
