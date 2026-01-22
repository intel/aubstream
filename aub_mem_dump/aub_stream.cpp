/*
 * Copyright (C) 2022-2026 Intel Corporation
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
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_entry_bits.h"
#include "aubstream/hint_values.h"

namespace aub_stream {

void AubStream::gttMemoryPoll(GGTT *ggtt, uint64_t gfxAddress, uint32_t value, uint32_t compareMode) {
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ggtt, gfxAddress, sizeof(value), PhysicalAddressAllocator::mainBank, ggtt->getPageSize(), PageTableWalker::WalkMode::Expect, nullptr);
    memoryPoll(pageWalker.entries, value, compareMode);
}

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

    std::vector<PageEntryInfo> pageWalkEntries[3];

    // Reserve minimal # of entries
    pageWalkEntries[PageTableLevel::Pte].reserve(2 + (uint64_t(size) / page64kSize));
    pageWalkEntries[PageTableLevel::Pde].reserve(1);
    pageWalkEntries[PageTableLevel::Pdp].reserve(1);

    while (size > 0) {
        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        PageTable *pdp = nullptr;
        PDE *pde = nullptr;
        PTE *pte = nullptr;

        bool is2MBPage = false;

        while (level >= 0) {
            if (!child) {
                return; // nothing to free.
            }
            parent = child;
            auto index = parent->getIndex(gfxAddress);

            // PDP is only valid if level == PageTableLevel::Pdp
            pdp = level == PageTableLevel::Pdp ? parent : pdp;
            // PDE is only valid if level == PageTableLevel::Pde
            pde = level == PageTableLevel::Pde ? static_cast<PDE *>(parent) : pde;
            // PTE is only valid if level == PageTableLevel::Pte
            pte = (level != PageTableLevel::Pte) ? nullptr : static_cast<PTE *>(parent);

            // Get each of the page structures
            child = parent->getChild(index);
            if (level == PageTableLevel::Pde && child) {
                // 2MB page found at PDE level
                if (child->getPageSize() == Page2MB::pageSize2MB) {
                    is2MBPage = true;
                    break;
                }
            }

            if (child && level == PageTableLevel::Pte) {
                assert(pte != nullptr);

                // remove page from PTE
                pte->setChild(index, nullptr);
                PageEntryInfo info = {
                    pte->getPhysicalAddress() + index * sizeof(uint64_t),
                    0,
                };
                pageWalkEntries[PageTableLevel::Pte].push_back(info);
            }
            --level;
        }

        size_t pageSizeThisIteration;

        if (is2MBPage) {
            assert(pde != nullptr);
            pageSizeThisIteration = Page2MB::pageSize2MB;

            // Remove 2MB page from PDE
            auto index = pde->getIndex(gfxAddress);
            pde->setChild(index, nullptr);
            PageEntryInfo info = {
                pde->getPhysicalAddress() + index * sizeof(uint64_t),
                0,
            };
            pageWalkEntries[PageTableLevel::Pde].push_back(info);

            // Delete 2MB page
            delete child;

            // Clean up empty PDE
            if (pdp && pde->isEmpty()) {
                auto pdpIndex = pdp->getIndex(gfxAddress);
                pdp->setChild(pdpIndex, nullptr);
                PageEntryInfo info1 = {
                    pdp->getPhysicalAddress() + pdpIndex * sizeof(uint64_t),
                    0,
                };
                pageWalkEntries[PageTableLevel::Pdp].push_back(info1);
                delete pde;
            }
        } else {
            assert(pte);
            pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
            assert(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536);

            // Delete physical page
            delete child;

            if (pde && pte->isEmpty()) {
                // remove PTE
                pde->setChild(pde->getIndex(gfxAddress), nullptr);
                PageEntryInfo info1 = {
                    pde->getPhysicalAddress() + pde->getIndex(gfxAddress) * sizeof(uint64_t),
                    0,
                };
                pageWalkEntries[PageTableLevel::Pde].push_back(info1);
                delete pte;
            }
        }

        auto pageOffset = gfxAddress & (pageSizeThisIteration - 1);
        auto sizeThisIteration = static_cast<size_t>(pageSizeThisIteration - pageOffset);
        sizeThisIteration = std::min(size, sizeThisIteration);

        gfxAddress += sizeThisIteration;
        size -= sizeThisIteration;
    }

    writePpgttLevel1(pageWalkEntries[PageTableLevel::Pte], ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());
    writePpgttLevel2(pageWalkEntries[PageTableLevel::Pde], ppgtt->isLocalMemory(), ppgtt->getNumAddressBits());
    // Write PDP level entries if any PDEs were removed
    if (!pageWalkEntries[PageTableLevel::Pdp].empty()) {
        auto addressSpace = ppgtt->isLocalMemory() ? AddressSpaceValues::TraceLocal : AddressSpaceValues::TraceNonlocal;
        writeDiscontiguousPages(pageWalkEntries[PageTableLevel::Pdp], addressSpace, DataTypeHintValues::TracePpgttLevel3);
    }
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
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pml4], AddressSpaceValues::TracePml4Entry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pdp], AddressSpaceValues::TracePhysicalPdpEntry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pde], AddressSpaceValues::TracePpgttPdEntry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pte], AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype);
    } else {
        auto addressSpace = pageTablesInLocalMemory ? AddressSpaceValues::TraceLocal : AddressSpaceValues::TraceNonlocal;
        if (usePml5) {
            writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pml5], addressSpace, DataTypeHintValues::TracePpgttLevel5);
        }
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pml4], addressSpace, DataTypeHintValues::TracePpgttLevel4);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pdp], addressSpace, DataTypeHintValues::TracePpgttLevel3);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pde], addressSpace, DataTypeHintValues::TracePpgttLevel2);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[PageTableLevel::Pte], addressSpace, DataTypeHintValues::TracePpgttLevel1);
    }
}

void AubStream::writePpgttLevel2(const std::vector<PageEntryInfo> &pageWalkEntry, bool pageTablesInLocalMemory, uint32_t numAddressBits) {
    bool usePml5 = numAddressBits > 48;
    bool useLegacyAddressSpaces = !usePml5 && !pageTablesInLocalMemory;

    if (useLegacyAddressSpaces) {
        writeDiscontiguousPages(pageWalkEntry, AddressSpaceValues::TracePpgttPdEntry, DataTypeHintValues::TraceNotype);
    } else {
        auto addressSpace = pageTablesInLocalMemory ? AddressSpaceValues::TraceLocal : AddressSpaceValues::TraceNonlocal;
        writeDiscontiguousPages(pageWalkEntry, addressSpace, DataTypeHintValues::TracePpgttLevel2);
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

bool AubStream::compareMemory(uint32_t readValue, uint32_t expectedValue, uint32_t compareOperation) {
    bool matches = false;

    switch (compareOperation) {
    case CmdServicesMemTraceMemoryPoll::ComparisonValues::Equal:
        matches = (readValue == expectedValue);
        break;
    case CmdServicesMemTraceMemoryPoll::ComparisonValues::NotEqual:
        matches = (readValue != expectedValue);
        break;
    case CmdServicesMemTraceMemoryPoll::ComparisonValues::Greater:
        matches = (readValue > expectedValue);
        break;
    case CmdServicesMemTraceMemoryPoll::ComparisonValues::GreaterEqual:
        matches = (readValue >= expectedValue);
        break;
    case CmdServicesMemTraceMemoryPoll::ComparisonValues::Less:
        matches = (readValue < expectedValue);
        break;
    case CmdServicesMemTraceMemoryPoll::ComparisonValues::LessEqual:
        matches = (readValue <= expectedValue);
        break;
    default:
        assert(false && "Unsupported compare mode");
        break;
    }

    return matches;
}

} // namespace aub_stream
