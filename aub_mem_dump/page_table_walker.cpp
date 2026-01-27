/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/page_table_walker.h"
#include "aub_mem_dump/memory_bank_helper.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/settings.h"
#include <cassert>

namespace aub_stream {

namespace {

inline size_t apply2MBFallback(size_t pageSize, uint64_t gfxAddress) {
    if (pageSize == Page2MB::pageSize2MB && (gfxAddress & (Page2MB::pageSize2MB - 1)) != 0) {
        PRINT_LOG_ERROR("Warning: 2MB page requested but gfxAddress 0x%llx is not 2MB-aligned. Falling back to 64KB pages.\n",
                        static_cast<unsigned long long>(gfxAddress));
        return 65536;
    }
    return pageSize;
}

inline bool detect2MBConflict(PageTable *child, int level, size_t pageSize) {
    return child && level == PageTableLevel::Pde &&
           pageSize == Page2MB::pageSize2MB &&
           child->getPageSize() != Page2MB::pageSize2MB;
}

} // namespace

void PageTableWalker::walkMemory(GGTT *ggtt, uint64_t gfxAddress, size_t size, uint32_t memoryBanks, size_t pageSize, WalkMode mode, const std::vector<PageInfo> *pageInfos) {
    auto &ggttEntries = pageWalkEntries[0];

    bool isLocalMemory = memoryBanks != PhysicalAddressAllocator::mainBank;

    entries.reserve(2 + (size / 4096));
    ggttEntries.reserve(2 + (size / 4096));

    while (size > 0) {
        auto pageOffset = gfxAddress & (pageSize - 1);
        auto gpuAddressAligned = uint32_t(gfxAddress - pageOffset);

        auto index = ggtt->getIndex(gpuAddressAligned);
        auto child = ggtt->getChild(index);
        if (child == nullptr) {
            assert(mode != WalkMode::Expect);

            child = ggtt->allocateChild(ggtt->getGpu(), pageSize, ggtt->getMemoryBank());

            ggtt->setChild(index, child);

            auto enableBits = isLocalMemory ? 3
                                            : 1;

            {
                PageEntryInfo writeInfo = {
                    ggtt->getEntryOffset() + sizeof(uint64_t) * index,
                    child->getPhysicalAddress() | enableBits};

                ggttEntries.push_back(writeInfo);
            }

            auto entriesToDump = pageSize / 4096;
            auto physAddress = child->getPhysicalAddress();
            for (auto i = 1u; i < entriesToDump; i++) {
                physAddress += 4096;
                ggtt->setChild(index + i, new PageTable(ggtt->getGpu(), physAddress, child->getMemoryBank()));

                PageEntryInfo writeInfo = {
                    ggtt->getEntryOffset() + sizeof(uint64_t) * (index + i),
                    physAddress | enableBits};
                ggttEntries.push_back(writeInfo);
            }
        }

        auto sizeThisPass = static_cast<size_t>(pageSize - pageOffset);
        sizeThisPass = std::min(sizeThisPass, size);

        // Record our PTE information
        PageInfo writeInfo = {
            child->getPhysicalAddress() + pageOffset,
            sizeThisPass,
            child->isLocalMemory()};
        entries.push_back(writeInfo);

        size -= sizeThisPass;
        gfxAddress += (uint32_t)sizeThisPass;
    }
}

void PageTableWalker::walkMemory(PageTable *ppgtt, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos) {
    auto size = allocationParams.size;
    if (size == 0) {
        return;
    }

    auto pageSize = allocationParams.pageSize;
    auto memoryBanks = allocationParams.memoryBanks;
    auto gfxAddress = allocationParams.gfxAddress;

    assert(pageSize > 0);

    pageSize = apply2MBFallback(pageSize, gfxAddress);

    const PageInfo *clonePageInfo = nullptr;
    uint32_t clonePageInfoIndex = 0;

    bool isLocalMemory = memoryBanks != PhysicalAddressAllocator::mainBank;
    size_t sizePageAligned = (size + pageSize - 1) & ~(pageSize - 1);
    MemoryBankHelper bankHelper(memoryBanks, gfxAddress & ~(pageSize - 1), sizePageAligned);

    // 2MB pages terminate at PDE level, smaller pages at PTE level
    int leafLevel = (pageSize == Page2MB::pageSize2MB) ? PageTableLevel::Pde : PageTableLevel::Pte;

    // Reserve # of entries plus two for leading/trailing pages
    pageWalkEntries[PageTableLevel::Pml5].reserve(2 + (uint64_t(size) >> 48));
    pageWalkEntries[PageTableLevel::Pml4].reserve(2 + (uint64_t(size) >> 39));
    pageWalkEntries[PageTableLevel::Pdp].reserve(2 + (uint64_t(size) >> 30));
    pageWalkEntries[PageTableLevel::Pde].reserve(2 + (uint64_t(size) >> 21));
    pageWalkEntries[PageTableLevel::Pte].reserve(2 + (uint64_t(size) / pageSize));
    pages64KB.reserve(2 + (uint64_t(size) / pageSize));
    entries.reserve(2 + (uint64_t(size) / pageSize));

    while (size > 0) {
        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        uint32_t pageMemoryBank = bankHelper.getMemoryBank(gfxAddress);

        if (mode == WalkMode::Clone) {
            clonePageInfo = &(*pageInfos)[clonePageInfoIndex++];
        }
        PTE *pte = nullptr;
        while (level >= leafLevel) {
            parent = child;
            auto index = parent->getIndex(gfxAddress);

            pte = (level == PageTableLevel::Pte) ? static_cast<PTE *>(parent) : nullptr;

            // Existing PTE determines actual page size
            if (pte && pte->getPageSize() != pageSize) {
                pageSize = pte->getPageSize();
            }

            // Get or allocate each of the page structures
            child = parent->getChild(index);

            // Handle pre-existing 2MB pages
            if (child && level == PageTableLevel::Pde && child->getPageSize() == Page2MB::pageSize2MB) {
                pageSize = Page2MB::pageSize2MB;
                leafLevel = PageTableLevel::Pde;
                break;
            }

            if (detect2MBConflict(child, level, pageSize)) {
                pageSize = 65536;
                leafLevel = PageTableLevel::Pte;
            }

            if (!child) {
                assert(mode != WalkMode::Expect);
                if (level == leafLevel && clonePageInfo) {
                    const auto physicalAddressAligned = clonePageInfo->physicalAddress & ~(static_cast<uint64_t>(pageSize - 1));
                    if (pageSize == Page2MB::pageSize2MB) {
                        child = new Page2MB(ppgtt->getGpu(), physicalAddressAligned, clonePageInfo->memoryBank, allocationParams.additionalParams);
                    } else {
                        child = new PageTableMemory(ppgtt->getGpu(), physicalAddressAligned, clonePageInfo->memoryBank, allocationParams.additionalParams);
                    }
                } else if (level != leafLevel) {
                    // For interior nodes, child use parent's memory bank
                    child = parent->allocateChild(ppgtt->getGpu(), pageSize, parent->getMemoryBank());
                } else {
                    // For leaf nodes, use pageMemoryBank which manages coloring
                    child = parent->allocateChild(ppgtt->getGpu(), pageSize, pageMemoryBank, allocationParams.additionalParams);
                }

                parent->setChild(index, child);

                PageEntryInfo info = {
                    parent->getPhysicalAddress() + index * sizeof(uint64_t),
                    child->getEntryValue()};
                pageWalkEntries[level].push_back(info);

                // Need to keep track of 64KB system pages
                if (level == PageTableLevel::Pte && mode == WalkMode::Reserve && !isLocalMemory && pageSize == 65536) {
                    pages64KB.push_back(child->getPhysicalAddress());
                }
            }
            --level;
        }

        // pte is null for 2MB pages
        size_t pageSizeThisIteration;
        if (pageSize == Page2MB::pageSize2MB) {
            pageSizeThisIteration = Page2MB::pageSize2MB;
        } else {
            assert(pte);
            pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
        }
        assert(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536 || pageSizeThisIteration == Page2MB::pageSize2MB);

        auto pageOffset = gfxAddress & (pageSizeThisIteration - 1);
        auto sizeThisIteration = static_cast<size_t>(pageSizeThisIteration - pageOffset);
        sizeThisIteration = std::min(size, sizeThisIteration);

        // Record page information
        PageInfo writeInfo = {
            child->getPhysicalAddress() + pageOffset,
            sizeThisIteration,
            child->isLocalMemory(),
            pageMemoryBank};
        entries.push_back(writeInfo);

        gfxAddress += sizeThisIteration;
        size -= sizeThisIteration;
    }
}

void PageTableWalker::walkMemory(PageTable *ppgtt, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos, uint64_t physicalAddress) {
    auto size = allocationParams.size;
    if (size == 0) {
        return;
    }

    auto pageSize = allocationParams.pageSize;
    auto pageMemoryBank = allocationParams.memoryBanks;
    auto gfxAddress = allocationParams.gfxAddress;

    assert(pageSize > 0);

    pageSize = apply2MBFallback(pageSize, gfxAddress);

    const PageInfo *clonePageInfo = nullptr;
    uint32_t clonePageInfoIndex = 0;

    bool isLocalMemory = pageMemoryBank != PhysicalAddressAllocator::mainBank;

    // 2MB pages terminate at PDE level, smaller pages at PTE level
    int leafLevel = (pageSize == Page2MB::pageSize2MB) ? PageTableLevel::Pde : PageTableLevel::Pte;

    // Reserve # of entries plus two for leading/trailing pages
    pageWalkEntries[PageTableLevel::Pml5].reserve(2 + (uint64_t(size) >> 48));
    pageWalkEntries[PageTableLevel::Pml4].reserve(2 + (uint64_t(size) >> 39));
    pageWalkEntries[PageTableLevel::Pdp].reserve(2 + (uint64_t(size) >> 30));
    pageWalkEntries[PageTableLevel::Pde].reserve(2 + (uint64_t(size) >> 21));
    pageWalkEntries[PageTableLevel::Pte].reserve(2 + (uint64_t(size) / pageSize));
    pages64KB.reserve(2 + (uint64_t(size) / pageSize));
    entries.reserve(2 + (uint64_t(size) / pageSize));

    while (size > 0) {
        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        if (mode == WalkMode::Clone) {
            clonePageInfo = &(*pageInfos)[clonePageInfoIndex++];
        }
        PTE *pte = nullptr;
        while (level >= leafLevel) {
            parent = child;
            auto index = parent->getIndex(gfxAddress);

            // PTE only exists at level 0
            pte = (level == PageTableLevel::Pte) ? static_cast<PTE *>(parent) : nullptr;

            // Existing PTE determines actual page size
            if (pte && pte->getPageSize() != pageSize) {
                pageSize = pte->getPageSize();
            }

            // Get or allocate each of the page structures
            child = parent->getChild(index);

            // Handle pre-existing 2MB pages
            if (child && level == PageTableLevel::Pde && child->getPageSize() == Page2MB::pageSize2MB) {
                pageSize = Page2MB::pageSize2MB;
                leafLevel = PageTableLevel::Pde;
                break;
            }

            if (detect2MBConflict(child, level, pageSize)) {
                pageSize = 65536;
                leafLevel = PageTableLevel::Pte;
            }

            if (!child) {
                assert(mode != WalkMode::Expect);
                if (level == leafLevel && clonePageInfo) {
                    const auto physicalAddressAligned = clonePageInfo->physicalAddress & ~(static_cast<uint64_t>(pageSize - 1));
                    if (pageSize == Page2MB::pageSize2MB) {
                        child = new Page2MB(ppgtt->getGpu(), physicalAddressAligned, clonePageInfo->memoryBank, allocationParams.additionalParams);
                    } else {
                        child = new PageTableMemory(ppgtt->getGpu(), physicalAddressAligned, clonePageInfo->memoryBank, allocationParams.additionalParams);
                    }
                } else if (level != leafLevel) {
                    // For interior nodes, child use parent's memory bank
                    child = parent->allocateChild(ppgtt->getGpu(), pageSize, parent->getMemoryBank());
                } else {
                    // for child node use memory bank previously reserved
                    child = parent->allocateChild(ppgtt->getGpu(), pageSize, pageMemoryBank, allocationParams.additionalParams, physicalAddress);
                }

                parent->setChild(index, child);
            }

            // When using PreReserved memory via explicit Map we want to override the PTEs for remapping cases
            if (level == leafLevel) {
                if (child->getPhysicalAddress() != physicalAddress) {
                    // We need to remap here
                    child->setPhysicalAddress(physicalAddress);
                }
                if (child->getMemoryBank() != pageMemoryBank) {
                    child->setMemoryBank(pageMemoryBank);
                }
            }

            PageEntryInfo info = {
                parent->getPhysicalAddress() + index * sizeof(uint64_t),
                child->getEntryValue()};
            pageWalkEntries[level].push_back(info);

            // Need to keep track of 64KB system pages
            if (level == PageTableLevel::Pte && mode == WalkMode::Reserve && !isLocalMemory && pageSize == 65536) {
                pages64KB.push_back(child->getPhysicalAddress());
            }

            --level;
        }

        // pte is null for 2MB pages
        size_t pageSizeThisIteration;
        if (pageSize == Page2MB::pageSize2MB) {
            pageSizeThisIteration = Page2MB::pageSize2MB;
        } else {
            assert(pte);
            pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
        }
        assert(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536 || pageSizeThisIteration == Page2MB::pageSize2MB);

        auto pageOffset = gfxAddress & (pageSizeThisIteration - 1);
        auto sizeThisIteration = static_cast<size_t>(pageSizeThisIteration - pageOffset);
        sizeThisIteration = std::min(size, sizeThisIteration);

        // Record page information
        PageInfo writeInfo = {
            child->getPhysicalAddress() + pageOffset,
            sizeThisIteration,
            child->isLocalMemory(),
            pageMemoryBank};
        entries.push_back(writeInfo);

        physicalAddress += sizeThisIteration;
        gfxAddress += sizeThisIteration;
        size -= sizeThisIteration;
    }
}
} // namespace aub_stream
