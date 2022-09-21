/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/page_table_walker.h"
#include "aub_mem_dump/memory_bank_helper.h"
#include <cassert>
#include <iostream>

namespace aub_stream {

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

    PageInfo clonePageInfo;
    uint32_t clonePageInfoIndex = 0;

    bool isLocalMemory = memoryBanks != PhysicalAddressAllocator::mainBank;
    size_t sizePageAligned = (size + pageSize - 1) & ~(pageSize - 1);
    MemoryBankHelper bankHelper(memoryBanks, gfxAddress & ~(pageSize - 1), sizePageAligned);

    // Reserve # of entries plus two for leading/trailing pages
    pageWalkEntries[4].reserve(2 + (uint64_t(size) >> 48));
    pageWalkEntries[3].reserve(2 + (uint64_t(size) >> 39));
    pageWalkEntries[2].reserve(2 + (uint64_t(size) >> 30));
    pageWalkEntries[1].reserve(2 + (uint64_t(size) >> 21));
    pageWalkEntries[0].reserve(2 + (uint64_t(size) / pageSize));
    pages64KB.reserve(2 + (uint64_t(size) / pageSize));
    entries.reserve(2 + (uint64_t(size) / pageSize));

    while (size > 0) {
        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        uint32_t pageMemoryBank = bankHelper.getMemoryBank(gfxAddress);

        if (mode == WalkMode::Clone) {
            clonePageInfo = (*pageInfos)[clonePageInfoIndex++];
        }

        PTE *pte = nullptr;
        while (level >= 0) {
            parent = child;
            auto index = parent->getIndex(gfxAddress);

            // PTE is only valid if level = 0
            pte = level ? nullptr : static_cast<PTE *>(parent);

            // Assert if pageSize conflicts with prior PTE
            if (pte && pte->getPageSize() != pageSize) {
                pageSize = pte->getPageSize();
            }

            // Get or allocate each of the page structures
            child = parent->getChild(index);
            if (!child) {
                assert(mode != WalkMode::Expect);

                if (level == 0 && mode == WalkMode::Clone) {
                    const auto physicalAddressAligned = clonePageInfo.physicalAddress & ~(static_cast<uint64_t>(pte->getPageSize() - 1));
                    child = new PageTableMemory(ppgtt->getGpu(), physicalAddressAligned, clonePageInfo.memoryBank, allocationParams.additionalParams);
                } else if (level != 0) {
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
                if (level == 0 && mode == WalkMode::Reserve && !isLocalMemory && pageSize == 65536) {
                    pages64KB.push_back(child->getPhysicalAddress());
                }
            }
            --level;
        }

        assert(pte);
        auto pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
        assert(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536);

        auto pageOffset = gfxAddress & (pageSizeThisIteration - 1);
        auto sizeThisIteration = static_cast<size_t>(pageSizeThisIteration - pageOffset);
        sizeThisIteration = std::min(size, sizeThisIteration);

        // Record our PTE information
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

    PageInfo clonePageInfo;
    uint32_t clonePageInfoIndex = 0;

    bool isLocalMemory = pageMemoryBank != PhysicalAddressAllocator::mainBank;

    // Reserve # of entries plus two for leading/trailing pages
    pageWalkEntries[4].reserve(2 + (uint64_t(size) >> 48));
    pageWalkEntries[3].reserve(2 + (uint64_t(size) >> 39));
    pageWalkEntries[2].reserve(2 + (uint64_t(size) >> 30));
    pageWalkEntries[1].reserve(2 + (uint64_t(size) >> 21));
    pageWalkEntries[0].reserve(2 + (uint64_t(size) / pageSize));
    pages64KB.reserve(2 + (uint64_t(size) / pageSize));
    entries.reserve(2 + (uint64_t(size) / pageSize));

    while (size > 0) {
        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        if (mode == WalkMode::Clone) {
            clonePageInfo = (*pageInfos)[clonePageInfoIndex++];
        }

        PTE *pte = nullptr;
        while (level >= 0) {
            parent = child;
            auto index = parent->getIndex(gfxAddress);

            // PTE is only valid if level = 0
            pte = level ? nullptr : static_cast<PTE *>(parent);

            // Assert if pageSize conflicts with prior PTE
            if (pte && pte->getPageSize() != pageSize) {
                pageSize = pte->getPageSize();
            }

            // Get or allocate each of the page structures
            child = parent->getChild(index);

            if (!child) {
                assert(mode != WalkMode::Expect);

                if (level == 0 && mode == WalkMode::Clone) {
                    const auto physicalAddressAligned = clonePageInfo.physicalAddress & ~(static_cast<uint64_t>(pte->getPageSize() - 1));
                    child = new PageTableMemory(ppgtt->getGpu(), physicalAddressAligned, clonePageInfo.memoryBank, allocationParams.additionalParams);
                } else if (level != 0) {
                    // For interior nodes, child use parent's memory bank
                    child = parent->allocateChild(ppgtt->getGpu(), pageSize, parent->getMemoryBank());
                } else {
                    // for child node use memory bank previously reserved
                    child = parent->allocateChild(ppgtt->getGpu(), pageSize, pageMemoryBank, allocationParams.additionalParams, physicalAddress);
                }

                parent->setChild(index, child);
            }

            // When using PreReserved memory via explicit Map we want to override the PTEs for remapping cases
            if (level == 0) {
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
            if (level == 0 && mode == WalkMode::Reserve && !isLocalMemory && pageSize == 65536) {
                pages64KB.push_back(child->getPhysicalAddress());
            }

            --level;
        }
        assert(pte);
        auto pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
        assert(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536);

        auto pageOffset = gfxAddress & (pageSizeThisIteration - 1);
        auto sizeThisIteration = static_cast<size_t>(pageSizeThisIteration - pageOffset);
        sizeThisIteration = std::min(size, sizeThisIteration);

        // Record our PTE information
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
