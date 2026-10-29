/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/page_table_walker.h"
#include "aub_mem_dump/debug_helpers.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_bank_helper.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/settings.h"
#include <unordered_set>

namespace aub_stream {

namespace {

inline size_t selectHardwareSupportedPageSize(const Gpu &gpu, size_t pageSize, uint32_t memoryBanks) {
    if (pageSize == Page2MB::pageSize2MB && !gpu.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize))) {
        return 65536;
    }
    return pageSize;
}

inline size_t adjustPageSizeForHardwareSupport(const Gpu &gpu, size_t pageSize, uint32_t memoryBanks) {
    const size_t supportedPageSize = selectHardwareSupportedPageSize(gpu, pageSize, memoryBanks);
    if (supportedPageSize != pageSize) {
        PRINT_LOG_ERROR("Warning: 2MB page requested but hardware does not support it for memoryBanks 0x%x. Falling back to 64KB pages.\n",
                        memoryBanks);
    }
    return supportedPageSize;
}

inline size_t getLeafPageSizeForExistingTables(PageTable *ppgtt, uint64_t gfxAddress, size_t requestedPageSize) {
    PageTable *node = ppgtt;
    for (int level = static_cast<int>(ppgtt->getNumLevels()) - 1; level > PageTableLevel::Pde; --level) {
        node = node->getChild(node->getIndex(gfxAddress));
        if (node == nullptr) {
            return requestedPageSize;
        }
    }
    const PageTable *leafTable = node->getChild(node->getIndex(gfxAddress));
    return leafTable ? leafTable->getPageSize() : requestedPageSize;
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

inline bool isPs64GroupEmpty(PageTable *pte, uint32_t hwBase) {
    for (uint32_t i = 0; i < Ps64::groupEntries; i++) {
        if (pte->getChild(hwBase + i) != nullptr) {
            return false;
        }
    }
    return true;
}

// Backs a naturally-aligned 16-entry PTE group with one 64KB-aligned
// physical block and marks every entry PS64. The entries share ownership
// of the block, which stays pending reservation until it reaches the streams.
inline void allocatePs64Group(PageTable *pte, uint32_t hwBase, uint32_t memoryBank, const AllocationParams::AdditionalParams &additionalAllocParams) {
    auto allocator = pte->getPhysicalAddressAllocator();
    const uint64_t base = allocator->reservePhysicalMemory(memoryBank, Ps64::groupSize, Ps64::groupSize);
    AUBSTREAM_DEBUG_ASSERT((base & (Ps64::groupSize - 1)) == 0);
    auto block = std::make_shared<SharedPhysicalBlock>(allocator, memoryBank, base);
    for (uint32_t i = 0; i < Ps64::groupEntries; i++) {
        auto entry = new PageTableMemory(pte->getGpu(), base + i * Ps64::pageSize, memoryBank, additionalAllocParams);
        entry->setSharedPhysicalBlock(block);
        entry->setPs64(true);
        entry->setPendingWrite(true);
        pte->setChild(hwBase + i, entry);
    }
}

// A PS64 translation needs the reservation and all 16 entries of the group, so visiting one
// pending entry emits every pending entry of its group, preceded by the outstanding reservation.
inline void emitPendingPs64Group(PageTable *pte, uint32_t index, std::vector<PageEntryInfo> &pteEntries,
                                 std::unordered_set<PageTable *> &emittedPending, std::vector<PageTable *> &pendingNodes,
                                 std::vector<uint64_t> &pages64KB, std::vector<SharedPhysicalBlock *> &pendingBlocks) {
    PageTable *visited = pte->getChild(index);
    if (emittedPending.count(visited)) {
        return;
    }
    const auto &block = visited->getSharedPhysicalBlock();
    if (block && block->reservationPending) {
        pages64KB.push_back(block->physicalAddress);
        pendingBlocks.push_back(block.get());
    }
    const uint32_t hwBase = index & ~(Ps64::groupEntries - 1);
    for (uint32_t i = hwBase; i < hwBase + Ps64::groupEntries; i++) {
        PageTable *entry = pte->getChild(i);
        if (entry == nullptr || !entry->isPs64() || !entry->isPendingWrite()) {
            continue;
        }
        if (emittedPending.insert(entry).second) {
            pendingNodes.push_back(entry);
            pteEntries.push_back({pte->getPhysicalAddress() + i * sizeof(uint64_t), entry->getEntryValue()});
        }
    }
}

// A remapped entry no longer belongs to its contiguous block, so the whole group
// loses PS64. Emits the other entries of the group - the caller emits the remapped one.
inline void clearPs64Group(PageTable *pte, uint32_t index, std::vector<PageEntryInfo> &pteEntries,
                           std::unordered_set<PageTable *> &emittedPending, std::vector<PageTable *> &pendingNodes) {
    const uint32_t hwBase = index & ~(Ps64::groupEntries - 1);
    for (uint32_t i = hwBase; i < hwBase + Ps64::groupEntries; i++) {
        PageTable *entry = pte->getChild(i);
        if (entry == nullptr || !entry->isPs64()) {
            continue;
        }
        entry->setPs64(false);
        entry->setPendingWrite(true);
        if (i == index) {
            continue;
        }
        pteEntries.push_back({pte->getPhysicalAddress() + i * sizeof(uint64_t), entry->getEntryValue()});
        if (emittedPending.insert(entry).second) {
            pendingNodes.push_back(entry);
        }
    }
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
            AUBSTREAM_DEBUG_ASSERT(mode != WalkMode::Expect);
            if (mode == WalkMode::Expect) {
                break;
            }

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

    AUBSTREAM_DEBUG_ASSERT(pageSize > 0);

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

    if (physicalAddress) {
        *physicalAddress += gfxAddress & (allocationParams.pageSize - 1);
    }

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

            const uint64_t leafPhysicalAddress = physicalAddress ? *physicalAddress - (gfxAddress & (pageSize - 1)) : 0;

            bool emitEntry = false;

            if (!child) {
                AUBSTREAM_DEBUG_ASSERT(mode != WalkMode::Expect);
                if (mode == WalkMode::Expect) {
                    break;
                }
                // PS64 is planned, not detected: only a walk that maps a whole, still empty,
                // 64KB-aligned VA window of 4KB system pages gets one fresh contiguous block.
                // Caller-provided physical memory is not known to be reserved as one contiguous block,
                // and cloned entries do not own the block they point to, so neither is promoted.
                const uint32_t hwBase = index & ~(Ps64::groupEntries - 1);
                const uint64_t pageStart = gfxAddress & ~static_cast<uint64_t>(Ps64::pageSize - 1);
                const bool planPs64Group = ps64Applicable && mode == WalkMode::Reserve && !physicalAddress &&
                                           !isLocalMemory && level == PageTableLevel::Pte && pte->getPageSize() == Ps64::pageSize &&
                                           index == hwBase && pageStart + Ps64::groupSize <= gfxAddress + size &&
                                           isPs64GroupEmpty(parent, hwBase);
                if (planPs64Group) {
                    allocatePs64Group(parent, hwBase, pageMemoryBank, allocationParams.additionalParams);
                    child = parent->getChild(index);
                } else if (level == leafLevel && clonePageInfo) {
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
                        child = parent->allocateChild(ppgtt->getGpu(), pageSize, pageMemoryBank, allocationParams.additionalParams, leafPhysicalAddress);
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
                if (child->getPhysicalAddress() != leafPhysicalAddress || child->getMemoryBank() != pageMemoryBank) {
                    // We need to remap here
                    if (child->isPs64()) {
                        clearPs64Group(parent, index, pageWalkEntries[level], emittedPending[level], pendingNodes[level]);
                    }
                    // The remapped entry no longer points into its PS64 block, so it must not keep it alive
                    child->setSharedPhysicalBlock(nullptr);
                    child->releaseOwnedPhysicalMemory();
                    child->setPhysicalAddress(leafPhysicalAddress);
                    child->setMemoryBank(pageMemoryBank);
                    child->setPendingWrite(true);
                    emitEntry = true;
                }
            }

            if (level == PageTableLevel::Pte && child->isPs64() && child->isPendingWrite()) {
                emitPendingPs64Group(parent, index, pageWalkEntries[level], emittedPending[level], pendingNodes[level], pages64KB, pendingBlocks);
                emitEntry = false;
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
            }
            --level;
        }

        if (child == nullptr) {
            break;
        }

        // pte is null for 2MB pages
        size_t pageSizeThisIteration;
        if (pageSize == Page2MB::pageSize2MB) {
            pageSizeThisIteration = Page2MB::pageSize2MB;
        } else {
            AUBSTREAM_DEBUG_ASSERT(pte);
            pageSizeThisIteration = pte->getPageSize(); // NOLINT(clang-analyzer-core.CallAndMessage)
        }
        AUBSTREAM_DEBUG_ASSERT(pageSizeThisIteration == 4096 || pageSizeThisIteration == 65536 || pageSizeThisIteration == Page2MB::pageSize2MB);

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
            *physicalAddress += sizeThisIteration;
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

std::optional<ExplicitMappingConflict> PageTableWalker::findExplicitMappingConflict(PageTable *ppgtt, const AllocationParams &allocationParams, uint64_t physicalAddress) {
    const size_t requestedPageSize = selectHardwareSupportedPageSize(ppgtt->getGpu(), allocationParams.pageSize, allocationParams.memoryBanks);
    uint64_t gfxAddress = allocationParams.gfxAddress;
    uint64_t physicalAddressAtGfxAddress = physicalAddress + (gfxAddress & (allocationParams.pageSize - 1));
    size_t remainingSize = allocationParams.size;

    while (remainingSize > 0) {
        const size_t leafPageSize = getLeafPageSizeForExistingTables(ppgtt, gfxAddress, requestedPageSize);
        const uint64_t pageOffsetMask = leafPageSize - 1;
        const uint64_t pageOffset = gfxAddress & pageOffsetMask;
        if ((physicalAddressAtGfxAddress & pageOffsetMask) != pageOffset) {
            return ExplicitMappingConflict{gfxAddress, physicalAddressAtGfxAddress, leafPageSize, (physicalAddressAtGfxAddress & ~pageOffsetMask) | pageOffset};
        }
        const size_t sizeThisIteration = std::min(remainingSize, static_cast<size_t>(leafPageSize - pageOffset));
        gfxAddress += sizeThisIteration;
        physicalAddressAtGfxAddress += sizeThisIteration;
        remainingSize -= sizeThisIteration;
    }
    return std::nullopt;
}

} // namespace aub_stream
