/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/page_table_walker.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_bank_helper.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/settings.h"
#include <cassert>
#include <unordered_set>

namespace aub_stream {

namespace {

inline size_t adjustPageSizeForHardwareSupport(const Gpu &gpu, size_t pageSize, uint32_t memoryBanks) {
    if (pageSize == Page2MB::pageSize2MB && !gpu.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize))) {
        PRINT_LOG_ERROR("Warning: 2MB page requested but hardware does not support it for memoryBanks 0x%x. Falling back to 64KB pages.\n",
                        memoryBanks);
        return 65536;
    }
    return pageSize;
}

inline bool hasExistingPageTableConflict(PageTable *child, int level, size_t pageSize) {
    if (child && level == PageTableLevel::Pde &&
        pageSize == Page2MB::pageSize2MB &&
        child->getPageSize() != Page2MB::pageSize2MB) {
        PRINT_LOG_ERROR("Warning: 2MB page requested but smaller page structure already exists at this PDE (pageSize=%zu). Using existing structure.\n",
                        child->getPageSize());
        return true;
    }
    return false;
}

inline uint64_t ps64EntryFlags(const PageTable *entry) {
    return (entry->getEntryValue() ^ entry->getPhysicalAddress()) & ~toBitValue(PpgttEntryBits::ps64Bit);
}

// Returns true when all 16 hardware PTE slots of the naturally-aligned 64KB group
// starting at hwBase are occupied by children with consecutive physical addresses
// and identical permission bits (PS64 spec: same perms, 64KB-aligned PA).
inline bool isPs64GroupComplete(PageTable *parent, uint32_t hwBase) {
    PageTable *c0 = parent->getChild(hwBase);
    if (!c0)
        return false;
    const uint64_t base = c0->getPhysicalAddress();
    if (base & 0xffff)
        return false;
    const uint64_t flags0 = ps64EntryFlags(c0);
    for (uint32_t i = 1; i < 16; i++) {
        PageTable *ci = parent->getChild(hwBase + i);
        if (!ci || ci->getPhysicalAddress() != base + i * 4096)
            return false; // must be consecutive physical addresses
        if (ps64EntryFlags(ci) != flags0)
            return false; // must have the same flags (e.g. permissions)
    }
    return true;
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

void PageTableWalker::walkMemory(PageTable *ppgtt, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos, std::optional<uint64_t> physicalAddress) {
    auto size = allocationParams.size;
    if (size == 0) {
        return;
    }

    auto pageSize = allocationParams.pageSize;
    auto memoryBanks = allocationParams.memoryBanks;
    auto gfxAddress = allocationParams.gfxAddress;

    assert(pageSize > 0);

    pageSize = adjustPageSizeForHardwareSupport(ppgtt->getGpu(), pageSize, memoryBanks);

    const PageInfo *clonePageInfo = nullptr;
    uint32_t clonePageInfoIndex = 0;

    bool isLocalMemory = memoryBanks != PhysicalAddressAllocator::mainBank;
    uint64_t alignedStart = gfxAddress & ~(static_cast<uint64_t>(pageSize - 1));
    uint64_t alignedEnd = (gfxAddress + size + pageSize - 1) & ~(static_cast<uint64_t>(pageSize - 1));
    size_t sizePageAligned = static_cast<size_t>(alignedEnd - alignedStart);
    MemoryBankHelper bankHelper(memoryBanks, alignedStart, sizePageAligned);

    // 2MB pages terminate at PDE level, smaller pages at PTE level
    int leafLevel = (pageSize == Page2MB::pageSize2MB) ? PageTableLevel::Pde : PageTableLevel::Pte;

    // Conflict fallback should only affect the PDE where it occurs
    const size_t requestedPageSize = pageSize;
    const int requestedLeafLevel = leafLevel;
    const bool ps64Applicable = globalSettings->EnablePs64.get() && requestedPageSize == 4096 && ppgtt->getNumLevels() == 5; // PS64 was introduced with 5-level PPGTT (PML5)

    // Reserve # of entries plus two for leading/trailing pages
    pageWalkEntries[PageTableLevel::Pml5].reserve(2 + (uint64_t(size) >> 48));
    pageWalkEntries[PageTableLevel::Pml4].reserve(2 + (uint64_t(size) >> 39));
    pageWalkEntries[PageTableLevel::Pdp].reserve(2 + (uint64_t(size) >> 30));
    pageWalkEntries[PageTableLevel::Pde].reserve(2 + (uint64_t(size) >> 21));
    pageWalkEntries[PageTableLevel::Pte].reserve(2 + (uint64_t(size) / pageSize));
    pages64KB.reserve(2 + (uint64_t(size) / pageSize));
    entries.reserve(2 + (uint64_t(size) / pageSize));

    // Per-walk set to emit each pending node at most once; interior nodes are shared across pages
    std::unordered_set<PageTable *> emittedPending[5];

    while (size > 0) {
        // Reset per-iteration. Conflict fallback is scoped to a single PDE
        pageSize = requestedPageSize;
        leafLevel = requestedLeafLevel;

        PageTable *parent = nullptr;
        PageTable *child = ppgtt;
        int level = ppgtt->getNumLevels() - 1;

        uint32_t pageMemoryBank = physicalAddress ? memoryBanks : bankHelper.getMemoryBank(gfxAddress);

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
                if (!physicalAddress) {
                    break;
                }
            }

            if (hasExistingPageTableConflict(child, level, pageSize)) {
                pageSize = 65536;
                leafLevel = PageTableLevel::Pte;
            }

            // Prior 64 KB fallback may have left physicalAddress unaligned
            if (physicalAddress && level == leafLevel && pageSize == Page2MB::pageSize2MB &&
                (*physicalAddress & (Page2MB::pageSize2MB - 1)) != 0) {
                *physicalAddress = (*physicalAddress & ~(Page2MB::pageSize2MB - 1)) + Page2MB::pageSize2MB;
            }

            // PS64: snapshot group state before this child is allocated/remapped
            const bool checkPs64 = ps64Applicable && level == PageTableLevel::Pte && pte != nullptr &&
                                   pte->getPageSize() == 4096;
            const uint32_t hwBase = checkPs64 ? (index & ~0xfu) : 0;
            const PageTable *c0 = checkPs64 ? parent->getChild(hwBase) : nullptr;
            const bool wasPs64Group = c0 != nullptr && c0->isPs64() && isPs64GroupComplete(parent, hwBase);
            bool emitEntry = false;

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
                    if (physicalAddress) {
                        child = parent->allocateChild(ppgtt->getGpu(), pageSize, pageMemoryBank, allocationParams.additionalParams, *physicalAddress);
                    } else {
                        child = parent->allocateChild(ppgtt->getGpu(), pageSize, pageMemoryBank, allocationParams.additionalParams);
                    }
                }

                parent->setChild(index, child);
                child->setPendingWrite(true);
                // Need to keep track of 64KB system pages
                if (level == PageTableLevel::Pte && mode == WalkMode::Reserve && !isLocalMemory && pageSize == 65536) {
                    pages64KB.push_back(child->getPhysicalAddress());
                }
                emitEntry = true;
            }

            // When using PreReserved memory via explicit Map we want to override the PTEs for remapping cases
            if (physicalAddress && level == leafLevel) {
                if (child->getPhysicalAddress() != *physicalAddress) {
                    // We need to remap here
                    child->setPhysicalAddress(*physicalAddress);
                    child->setPendingWrite(true);
                    emitEntry = true;
                }
                if (child->getMemoryBank() != pageMemoryBank) {
                    child->setMemoryBank(pageMemoryBank);
                    child->setPendingWrite(true);
                    emitEntry = true;
                }
            }

            if (checkPs64) {
                if (isPs64GroupComplete(parent, hwBase)) {
                    // Group is now complete - emit slots not yet written as PS64 or pending stream commit
                    for (uint32_t i = 0; i < 16; i++) {
                        PageTable *ci = parent->getChild(hwBase + i);
                        bool isNewPs64 = !ci->isPs64();
                        if (isNewPs64) {
                            ci->setPs64(true);
                            ci->setPendingWrite(true);
                        }
                        bool isNewPending = ci->isPendingWrite() && emittedPending[level].insert(ci).second;
                        // isNewPs64=true with insert=false is intentional: a wasPs64Group downgrade in an
                        // earlier iteration inserted ci into emittedPending (non-PS64 entry already emitted),
                        // and this completion pass re-emits ci with the PS64 bit set. The stream receives
                        // the updated value; the node is already tracked in pendingNodes for clearing.
                        if (isNewPs64 || isNewPending) {
                            pageWalkEntries[level].push_back({parent->getPhysicalAddress() + (hwBase + i) * sizeof(uint64_t),
                                                              ci->getEntryValue()});
                        }
                        if (isNewPending) {
                            pendingNodes[level].push_back(ci);
                        }
                    }
                    emitEntry = false;
                } else if (wasPs64Group) {
                    // Group was PS64-complete but is no longer consecutive - clear PS64 on each leaf
                    for (uint32_t i = 0; i < 16; i++) {
                        PageTable *ci = parent->getChild(hwBase + i);
                        assert(ci != nullptr);
                        ci->setPs64(false);
                        ci->setPendingWrite(true);
                        pageWalkEntries[level].push_back({parent->getPhysicalAddress() + (hwBase + i) * sizeof(uint64_t),
                                                          ci->getEntryValue()});
                        if (emittedPending[level].insert(ci).second) {
                            pendingNodes[level].push_back(ci);
                        }
                    }
                    emitEntry = false;
                } else {
                    const uint32_t k = index - hwBase;
                    const uint64_t expectedPhysBase = child->getPhysicalAddress() - static_cast<uint64_t>(k) * 4096;
                    const bool willComplete = (expectedPhysBase & 0xffff) == 0 &&
                                              size >= static_cast<size_t>(16 - k) * 4096 &&
                                              (k == 0 || (c0 != nullptr &&
                                                          c0->getPhysicalAddress() == expectedPhysBase &&
                                                          ps64EntryFlags(c0) == ps64EntryFlags(child)));
                    if (willComplete) {
                        child->setPs64(true);
                        child->setPendingWrite(true);
                    }
                    emitEntry = true;
                }
            } else {
                emitEntry = emitEntry || physicalAddress.has_value();
                if (child->isPendingWrite()) {
                    if (emittedPending[level].insert(child).second) {
                        pendingNodes[level].push_back(child);
                        emitEntry = true;
                    }
                }
            }
            if (emitEntry) {
                pageWalkEntries[level].push_back({parent->getPhysicalAddress() + index * sizeof(uint64_t),
                                                  child->getEntryValue()});
                if (checkPs64 && child->isPendingWrite() && emittedPending[level].insert(child).second) {
                    pendingNodes[level].push_back(child);
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

        if (physicalAddress) {
            // 2 MB Pages consume a full 2MB-aligned physical page
            if (pageSizeThisIteration == Page2MB::pageSize2MB) {
                *physicalAddress = (*physicalAddress & ~(Page2MB::pageSize2MB - 1)) + Page2MB::pageSize2MB;
            } else {
                *physicalAddress += sizeThisIteration;
            }
        }
        gfxAddress += sizeThisIteration;
        size -= sizeThisIteration;
    }
}

void PageTableWalker::walkMemory(PageTable *ppgtt, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos) {
    walkMemory(ppgtt, allocationParams, mode, pageInfos, std::nullopt);
}

void PageTableWalker::walkMemory(PageTable *ppgtt, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos, uint64_t physicalAddress) {
    walkMemory(ppgtt, allocationParams, mode, pageInfos, std::make_optional(physicalAddress));
}

} // namespace aub_stream
