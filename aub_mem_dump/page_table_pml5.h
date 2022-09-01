/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/page_table.h"

namespace aub_stream {

struct PML5 : public PageTable {
    using AddressType = uint64_t;

    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 48) & 0x1ff;
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        assert(pageTableMemoryBank == memoryBank);
        return new PML4(gpu, allocator, pageTableMemoryBank);
    }

    PML5(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PageTable(gpu, physicalAddressAllocator, 4096u, 512u, memoryBank) {
        numAddressBits = 57;
        numLevels = 5;
    }
};

} // namespace aub_stream
