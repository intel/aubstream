/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <algorithm>
#include <cassert>
#include <sstream>
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/memory_bank_helper.h"
#include "aub_mem_dump/page_table_entry_bits.h"

namespace aub_stream {

void AubStream::expectMemory(GGTT *ggtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation) {
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ggtt, gfxAddress, size, PhysicalAddressAllocator::mainBank, ggtt->getPageSize(), PageTableWalker::WalkMode::Expect, nullptr);
    expectMemoryTable(memory, size, pageWalker.entries, compareOperation);
}

void AubStream::expectMemory(PageTable *ppgtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation) {
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt, {gfxAddress, nullptr, size, PhysicalAddressAllocator::mainBank, 0, 65536}, PageTableWalker::WalkMode::Expect, nullptr);
    expectMemoryTable(memory, size, pageWalker.entries, compareOperation);
}

void AubStream::readMemory(PageTable *ppgtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize) {
    assert(ppgtt->getNumLevels() > 1);
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt, {gfxAddress, nullptr, size, memoryBanks, 0, pageSize}, PageTableWalker::WalkMode::Reserve, nullptr);
    readDiscontiguousPages(memory, size, pageWalker.entries);
}

void AubStream::readMemory(GGTT *ggtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize) {
    PageTableWalker pageWalker;

    pageWalker.walkMemory(ggtt, gfxAddress, size, memoryBanks, pageSize, PageTableWalker::WalkMode::Reserve, nullptr);
    readDiscontiguousPages(memory, size, pageWalker.entries);
}

std::vector<PageInfo> AubStream::writeMemory(GGTT *ggtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) {
    std::ostringstream str;
    str << "ggtt: " << std::hex << std::showbase << gfxAddress
        << " - " << gfxAddress + size - 1
        << "  pageSize: " << pageSize
        << "  banks:"
        << memoryBanksToString(memoryBanks, " ");

    addComment(str.str().c_str());

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ggtt, gfxAddress, size, memoryBanks, pageSize, PageTableWalker::WalkMode::Reserve, nullptr);

    writeGttPages(ggtt, pageWalker.pageWalkEntries[0]);

    if (memory) {
        writeDiscontiguousPages(memory, size, pageWalker.entries, hint);
    }
    return pageWalker.entries;
}

std::vector<PageInfo> AubStream::writeMemory(PageTable *ppgtt, const AllocationParams &allocationParams) {
    auto memory = allocationParams.memory;
    auto gfxAddress = allocationParams.gfxAddress;
    auto size = allocationParams.size;
    std::ostringstream str;
    str << "ppgtt" << (memory == nullptr ? "(pages only): " : ":") << std::hex << std::showbase << gfxAddress
        << " - " << gfxAddress + size - 1
        << "  pageSize: " << allocationParams.pageSize
        << "  banks:"
        << memoryBanksToString(allocationParams.memoryBanks, " ");

    addComment(str.str().c_str());

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt, allocationParams, PageTableWalker::WalkMode::Reserve, nullptr);

    writePages(pageWalker, memory, size, allocationParams.hint, ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());

    return pageWalker.entries;
}

void AubStream::writeMemoryAndClonePageTables(PageTable *ppgtt, PageTable *ppgttForCloning[], uint32_t ppggtForCloningCount, uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) {
    std::ostringstream str;
    str << "ppgtt: " << std::hex << std::showbase << gfxAddress
        << " - " << gfxAddress + size - 1
        << "  pageSize: " << pageSize
        << "  banks:"
        << memoryBanksToString(memoryBanks, " ");

    addComment(str.str().c_str());

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt, {gfxAddress, memory, size, memoryBanks, hint, pageSize}, PageTableWalker::WalkMode::Reserve, nullptr);

    writePages(pageWalker, memory, size, hint, ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());

    for (uint32_t i = 0; i < ppggtForCloningCount; i++) {
        cloneMemory(ppgttForCloning[i], pageWalker.entries, {gfxAddress, memory, size, memoryBanks, hint, pageSize});
    }
}

void AubStream::cloneMemory(PageTable *ppgtt, const std::vector<PageInfo> &entries, const AllocationParams &allocationParams) {
    assert(ppgtt->getNumLevels() > 1);
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt, allocationParams, PageTableWalker::WalkMode::Clone, &entries);

    writePages(pageWalker, nullptr, 0, 0, ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());
}

void AubStream::freeMemory(PageTable *ppgtt, uint64_t gfxAddress, size_t size) {
    assert(ppgtt->getNumLevels() > 1);
    assert(size != 0);
    constexpr size_t page64kSize = 65536u;

    std::vector<PageEntryInfo> pageWalkEntries[1];

    // Reserve minimal # of entries
    pageWalkEntries[0].reserve(2 + (uint64_t(size) / page64kSize));

    while (size > 0) {
        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        PTE *pte = nullptr;
        while (level >= 0) {
            if (!child) {
                return; // nothing to free.
            }
            parent = child;
            auto index = parent->getIndex(gfxAddress);

            // PTE is only valid if level = 0
            pte = level ? nullptr : static_cast<PTE *>(parent);

            // Get each of the page structures
            child = parent->getChild(index);

            if (child && level == 0) {
                assert(pte != nullptr);

                // remove page from PTE
                pte->setChild(index, nullptr);
                PageEntryInfo info = {
                    parent->getPhysicalAddress() + index * sizeof(uint64_t),
                    0,
                };
                pageWalkEntries[0].push_back(info);
            }
            --level;
        }

        assert(pte);
        auto pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
        assert(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536);

        // Delete physical page
        delete child;

        auto pageOffset = gfxAddress & (pageSizeThisIteration - 1);
        auto sizeThisIteration = static_cast<size_t>(pageSizeThisIteration - pageOffset);
        sizeThisIteration = std::min(size, sizeThisIteration);

        gfxAddress += sizeThisIteration;
        size -= sizeThisIteration;
    }

    writePpgttLevel1(pageWalkEntries[0], ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());
}

void AubStream::writePages(const PageTableWalker &pageWalker, const void *memory, size_t size, int hint, bool pageTablesInLocalMemory,
                           uint32_t numAddressBits) {
    if (pageWalker.pages64KB.size() > 0) {
        // For 64KB pages, we have to reserve those regions prior to the dumping the PTEntries to ensure we get 64KB pages.
        reserveContiguousPages(pageWalker.pages64KB);
    }

    writePageWalkEntries(pageWalker, pageTablesInLocalMemory, numAddressBits);

    if (memory) {
        writeDiscontiguousPages(memory, size, pageWalker.entries, hint);
    }
}

void AubStream::writePhysicalMemoryPages(const void *memory, size_t size, const std::vector<PageInfo> entries, int hint) {
    std::ostringstream str;
    str << "memory only (without page tables entries): "
        << std::hex << std::showbase << uint64_t(memory) << " - " << (uint64_t)memory + size - 1;

    addComment(str.str().c_str());
    writeDiscontiguousPages(memory, size, entries, hint);
}

bool AubStream::mapGpuVa(PageTable *ppgtt, AllocationParams allocationParams, uint64_t physicalAddress) {
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt, allocationParams, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    writePages(pageWalker, nullptr, 0, allocationParams.hint, ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());

    return true;
}

void AubStream::writePageWalkEntries(const PageTableWalker &pageWalker, bool pageTablesInLocalMemory, uint32_t numAddressBits) {
    bool usePml5 = numAddressBits > 48;
    bool useLegacyAddressSpaces = !usePml5 && !pageTablesInLocalMemory;

    if (useLegacyAddressSpaces) {
        writeDiscontiguousPages(pageWalker.pageWalkEntries[3], AddressSpaceValues::TracePml4Entry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[2], AddressSpaceValues::TracePhysicalPdpEntry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[1], AddressSpaceValues::TracePpgttPdEntry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[0], AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype);
    } else {
        auto addressSpace = pageTablesInLocalMemory ? AddressSpaceValues::TraceLocal : AddressSpaceValues::TraceNonlocal;
        if (usePml5) {
            writeDiscontiguousPages(pageWalker.pageWalkEntries[4], addressSpace, DataTypeHintValues::TracePpgttLevel5);
        }
        writeDiscontiguousPages(pageWalker.pageWalkEntries[3], addressSpace, DataTypeHintValues::TracePpgttLevel4);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[2], addressSpace, DataTypeHintValues::TracePpgttLevel3);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[1], addressSpace, DataTypeHintValues::TracePpgttLevel2);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[0], addressSpace, DataTypeHintValues::TracePpgttLevel1);
    }
}

void AubStream::writePpgttLevel1(const std::vector<PageEntryInfo> &pageWalkEntry, bool pageTablesInLocalMemory, uint32_t numAddressBits) {
    bool usePml5 = numAddressBits > 48;
    bool useLegacyAddressSpaces = !usePml5 && !pageTablesInLocalMemory;

    if (useLegacyAddressSpaces) {
        writeDiscontiguousPages(pageWalkEntry, AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype);
    } else {
        auto addressSpace = pageTablesInLocalMemory ? AddressSpaceValues::TraceLocal : AddressSpaceValues::TraceNonlocal;
        writeDiscontiguousPages(pageWalkEntry, addressSpace, DataTypeHintValues::TracePpgttLevel1);
    }
}

} // namespace aub_stream
