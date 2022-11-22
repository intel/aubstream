/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include <vector>
#include "physical_address_allocator.h"
#include "page_table_entry_bits.h"
#include "aubstream/allocation_params.h"

namespace aub_stream {

struct Gpu;

struct PageTable {

    PageTable(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, size_t size, unsigned int tableCount, uint32_t memoryBank);

    PageTable(const Gpu &gpu, uint64_t physicalAddress, uint32_t memoryBank)
        : PageTable(gpu, nullptr, 0, 0, memoryBank) {
        this->physicalAddress = physicalAddress;
    }

    PageTable(const Gpu &gpu, uint64_t physicalAddress, uint32_t memoryBank, const AllocationParams::AdditionalParams &additionalAllocParams)
        : PageTable(gpu, physicalAddress, memoryBank) {
        this->additionalAllocParams = additionalAllocParams;
    }

    PageTable(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, size_t size, unsigned int tableCount, uint32_t memoryBank, const AllocationParams::AdditionalParams &additionalAllocParams)
        : PageTable(gpu, physicalAddressAllocator, size, tableCount, memoryBank) {
        this->additionalAllocParams = additionalAllocParams;
    }

    virtual ~PageTable();

    virtual unsigned int getIndex(uint64_t gfxAddress) {
        return 0;
    }

    virtual PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) {
        return nullptr;
    }

    virtual PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank, const AllocationParams::AdditionalParams &additionalAllocParams) {
        return nullptr;
    }

    virtual PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank, const AllocationParams::AdditionalParams &additionalAllocParams, uint64_t physicalAddress) {
        return nullptr;
    }

    PageTable *getChild(unsigned int index) {
        if (table.size() <= index) {
            return nullptr;
        }
        return table[index];
    }

    void setChild(unsigned int index, PageTable *child) {
        if (table.size() <= index) {
            table.resize(index + 1);
        }
        table[index] = child;
    }

    bool isLocalMemory() const {
        return memoryBank != 0;
    }

    virtual uint64_t getEntryValue() const;

    uint64_t getPhysicalAddress() const {
        return physicalAddress;
    }

    void setPhysicalAddress(uint64_t physicalAddressIn) {
        physicalAddress = physicalAddressIn;
    }

    PhysicalAddressAllocator *getPhysicalAddressAllocator() {
        return allocator;
    }

    uint32_t getMemoryBank() const {
        return memoryBank;
    }

    void setMemoryBank(uint32_t memoryBankIn) {
        memoryBank = memoryBankIn;
    }

    uint32_t getNumAddressBits() const {
        return numAddressBits;
    }

    uint32_t getNumLevels() const {
        return numLevels;
    }

    const Gpu &getGpu() const {
        return gpu;
    }

    const AllocationParams::AdditionalParams &peekAllocationParams() const {
        return additionalAllocParams;
    }

  protected:
    const Gpu &gpu;
    PhysicalAddressAllocator *allocator;
    AllocationParams::AdditionalParams additionalAllocParams = {};
    uint64_t physicalAddress;
    uint32_t memoryBank = 0;
    std::vector<PageTable *> table;

    // Only defined for PML4/PDP4/GGTT
    uint32_t numAddressBits = 0;
    uint32_t numLevels = 0;
};

using PageTableMemory = PageTable;

template <typename PageTableMemoryType>
struct PTEBase : public PageTable {
    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        return new PageTableMemoryType(gpu, allocator, pageSize, 0u, pageTableMemoryBank);
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank, const AllocationParams::AdditionalParams &additionalAllocParams) override {
        return new PageTableMemoryType(gpu, allocator, pageSize, 0u, pageTableMemoryBank, additionalAllocParams);
    }

    // Allocate child at pre-reserved physical address
    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank, const AllocationParams::AdditionalParams &additionalAllocParams, uint64_t physicalAddress) override {
        return new PageTableMemoryType(gpu, physicalAddress, pageTableMemoryBank, additionalAllocParams);
    }

    PTEBase(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PageTable(gpu, physicalAddressAllocator, 4096u, 512u, memoryBank) {
    }

    PTEBase(const Gpu &gpu, uint64_t physicalAddress, uint32_t memoryBank)
        : PageTable(gpu, physicalAddress, memoryBank) {
    }

    virtual size_t getPageSize() const = 0;
};

using PTE = PTEBase<PageTableMemory>;

struct LegacyPTE64KB : public PTE {
    LegacyPTE64KB(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PTE(gpu, physicalAddressAllocator, memoryBank) {
    }

    LegacyPTE64KB(const Gpu &gpu, uint64_t physicalAddress, uint32_t memoryBank)
        : PTE(gpu, physicalAddress, memoryBank) {
    }

    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::legacyIntermediatePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 12) & 0x1f0;
    }

    size_t getPageSize() const override {
        return 65536u;
    }
};

struct PTE64KB : public PTE {
    PTE64KB(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PTE(gpu, physicalAddressAllocator, memoryBank) {
    }

    PTE64KB(const Gpu &gpu, uint64_t physicalAddress, uint32_t memoryBank)
        : PTE(gpu, physicalAddress, memoryBank) {
    }

    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::intermediatePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 16) & 0x1f;
    }

    size_t getPageSize() const override {
        return 65536u;
    }
};

struct PTE4KB : public PTE {
    PTE4KB(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PTE(gpu, physicalAddressAllocator, memoryBank) {
    }

    PTE4KB(const Gpu &gpu, uint64_t physicalAddress, uint32_t memoryBank)
        : PTE(gpu, physicalAddress, memoryBank) {
    }

    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 12) & 0x1ff;
    }

    size_t getPageSize() const override {
        return 4096u;
    }
};

template <typename ChildType4KB, typename ChildType64KB>
struct PDEBase : public PageTable {
    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 21) & 0x1ff;
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        assert(pageTableMemoryBank == memoryBank);
        if (pageSize == 65536u) {
            return new ChildType64KB(gpu, allocator, pageTableMemoryBank);
        } else {
            return new ChildType4KB(gpu, allocator, pageTableMemoryBank);
        }
    }

    PDEBase(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PageTable(gpu, physicalAddressAllocator, 4096u, 512u, memoryBank) {
    }
};

using PDE = PDEBase<PTE4KB, PTE64KB>;
using LegacyPDE = PDEBase<PTE4KB, LegacyPTE64KB>;

template <typename ChildType>
struct PDPBase : public PageTable {
    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 30) & 0x1ff;
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        assert(pageTableMemoryBank == memoryBank);
        return new ChildType(gpu, allocator, pageTableMemoryBank);
    }

    PDPBase(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PageTable(gpu, physicalAddressAllocator, 4096u, 512u, memoryBank) {
    }
};

using PDP = PDPBase<PDE>;
using LegacyPDP = PDPBase<LegacyPDE>;

template <typename ChildType>
struct PML4Base : public PageTable {
    using AddressType = uint64_t;

    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 39) & 0x1ff;
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        assert(pageTableMemoryBank == memoryBank);
        return new ChildType(gpu, allocator, pageTableMemoryBank);
    }

    PML4Base(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PageTable(gpu, physicalAddressAllocator, 4096u, 512u, memoryBank) {
        numAddressBits = 48;
        numLevels = 4;
    }
};

using PML4 = PML4Base<PDP>;
using LegacyPML4 = PML4Base<LegacyPDP>;

template <typename ChildType>
struct PDP4Base : public PageTable {
    using AddressType = uint32_t;

    uint64_t getEntryValue() const override {
        return getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    }

    unsigned int getIndex(uint64_t gfxAddress) override {
        return (gfxAddress >> 30) & 0x3;
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        assert(pageTableMemoryBank == memoryBank);
        return new ChildType(gpu, allocator, pageTableMemoryBank);
    }

    PDP4Base(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank)
        : PageTable(gpu, physicalAddressAllocator, 0u, 4u, memoryBank) {
        numAddressBits = 32;
        numLevels = 3;
        PageTable::setChild(0, PDP4Base::allocateChild(gpu, 4096, memoryBank));
        PageTable::setChild(1, PDP4Base::allocateChild(gpu, 4096, memoryBank));
        PageTable::setChild(2, PDP4Base::allocateChild(gpu, 4096, memoryBank));
        PageTable::setChild(3, PDP4Base::allocateChild(gpu, 4096, memoryBank));
    }
};

using PDP4 = PDP4Base<PDE>;
using LegacyPDP4 = PDP4Base<LegacyPDE>;

struct GGTT : public PageTable {
    using AddressType = uint32_t;

    GGTT(const Gpu &gpu, PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress = 0);

    unsigned int getIndex(uint64_t gfxAddress) override {
        return uint32_t(gfxAddress) >> 12;
    }

    PageTable *allocateChild(const Gpu &gpu, size_t pageSize, uint32_t pageTableMemoryBank) override {
        assert(pageTableMemoryBank == memoryBank);
        return new PageTableMemory(gpu, allocator, pageSize, 0u, pageTableMemoryBank);
    }

    size_t getPageSize() const {
        return 4096u;
    }

    uint64_t entryOffset;

    auto getEntryOffset() const -> decltype(entryOffset) {
        return entryOffset;
    }

    SimpleAllocator<uint32_t> gfxAddressAllocator;
    uint64_t gttTableOffset; // An offset into either System Memory or Local Memory base
};

} // namespace aub_stream
