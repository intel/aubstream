/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "aub_mem_dump/page_table_walker.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "test.h"
#include "gtest/gtest.h"
#include "variable_backup.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::Return;

template <typename Type, uint32_t memoryBanksCount = 0>
struct PageTableWalkerFixture : public ::testing::Test {
    using PPGTTType = Type;

    void SetUp() override {
        allocator = std::make_unique<PhysicalAddressAllocatorSimple>(memoryBanksCount, aub_stream::GB, true);
        ggtt = std::make_unique<GGTT>(*gpu, allocator.get(), defaultMemoryBank);
        ppgtt = std::make_unique<PPGTTType>(*gpu, allocator.get(), defaultMemoryBank);
    }

    static void simulateWritePageWalkEntries(const PageTableWalker &walker) {
        for (const auto &levelNodes : walker.pendingNodes)
            for (auto *node : levelNodes)
                node->setPendingWrite(false);
    }

    std::unique_ptr<PhysicalAddressAllocatorSimple> allocator;
    std::unique_ptr<GGTT> ggtt;
    std::unique_ptr<PPGTTType> ppgtt;
};

using PageTableWalkerTest = PageTableWalkerFixture<PML4>;

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForLocalMemoryThenPageTableWalkerHasNo64KBPagesAndCorrectPageWalkEntriesAndPageEntriesStored) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);

    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // (1<<39)-4 with a 65536-byte page crosses the PML4 boundary after 4 bytes - 2 entries at every level
    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(2u, pageWalker.entries.size());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForSystem64KBMemoryThenPageTableWalkerHas64KBPagesAndCorrectPageWalkEntriesAndPageEntriesStored) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);

    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_SYSTEM, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // (1<<39)-4 with a 65536-byte page crosses the PML4 boundary after 4 bytes - 2 entries at every level
    EXPECT_EQ(2u, pageWalker.pages64KB.size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(2u, pageWalker.entries.size());
}

TEST_F(PageTableWalkerTest, givenCloneModeAndPPGTTWhenWalkingMemoryForSystem64KBMemoryThenPageTableWalkerHasNo64KBPagesAndPageWalkEntriesAndTwoPageEntriesStored) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);

    PageTableWalker pageWalkerReserve;
    pageWalkerReserve.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_SYSTEM, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    simulateWritePageWalkEntries(pageWalkerReserve);

    std::vector<PageInfo> pageInfosForCloning = pageWalkerReserve.entries;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_SYSTEM, 0, 65536}, PageTableWalker::WalkMode::Clone, &pageInfosForCloning);

    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(2u, pageWalker.entries.size());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndGGTTWhenWalkingMemoryForLocalMemoryThenPageTableWalkerHasNo64KBPagesAndOnePageWalkEntry0AndOnePageEntryStored) {
    const uint64_t gfxAddress = 0x23000;
    uint32_t data = 0xabcdabcd;

    PageTableWalker pageWalker;

    pageWalker.walkMemory(ggtt.get(), gfxAddress, sizeof(data), MEMORY_BANK_0, 4096, PageTableWalker ::WalkMode::Reserve, nullptr);

    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[0].size()); // GGTT
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(1u, pageWalker.entries.size());
}

// Tests for PreReserved Memory

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForLocalMemoryThenPageTableWalkerHasNo64KBPagesAndCorrectPageWalkEntriesAndPageEntriesStoredWithPreReserved) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);
    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(2u, pageWalker.entries.size());

    EXPECT_EQ(physicalAddress + (gfxAddress & (65536u - 1)), pageWalker.entries[0].physicalAddress);
    EXPECT_EQ(physicalAddress + (65536u - (gfxAddress & (65536u - 1))), pageWalker.entries[1].physicalAddress);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForSystem64KBMemoryThenPageTableWalkerHas64KBPagesAndCorrectPageWalkEntriesAndPageEntriesStoredWithPreReserved) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);
    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_SYSTEM, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    EXPECT_EQ(2u, pageWalker.pages64KB.size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(2u, pageWalker.entries.size());

    EXPECT_EQ(physicalAddress + (gfxAddress & (65536u - 1)), pageWalker.entries[0].physicalAddress);
    EXPECT_EQ(physicalAddress + (65536u - (gfxAddress & (65536u - 1))), pageWalker.entries[1].physicalAddress);
}

TEST_F(PageTableWalkerTest, givenCloneModeAndPPGTTWhenWalkingMemoryForSystem64KBMemoryThenPageTableWalkerHasNo64KBPagesAndPageWalkEntriesAndTwoPageEntriesStoredWithPreReserved) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    PageTableWalker pageWalkerReserve;
    pageWalkerReserve.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_SYSTEM, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    simulateWritePageWalkEntries(pageWalkerReserve);

    std::vector<PageInfo> pageInfosForCloning = pageWalkerReserve.entries;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 65536, MEMORY_BANK_SYSTEM, 0, 65536}, PageTableWalker::WalkMode::Clone, &pageInfosForCloning);

    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(2u, pageWalker.entries.size());

    EXPECT_EQ(physicalAddress + (gfxAddress & (65536u - 1)), pageWalker.entries[0].physicalAddress);
    EXPECT_EQ(physicalAddress + (65536u - (gfxAddress & (65536u - 1))), pageWalker.entries[1].physicalAddress);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForLocalMemoryForTheSameAddressTwiceReturnsTheSamePageWalkerSizes) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);
    const size_t size = 2 * 65536u;

    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, size, 65536);
    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(3u, pageWalker.entries.size());

    EXPECT_EQ(physicalAddress + (gfxAddress & (65536u - 1)), pageWalker.entries[0].physicalAddress);
    EXPECT_EQ(physicalAddress + (65536u - (gfxAddress & (65536u - 1))), pageWalker.entries[1].physicalAddress);

    PageTableWalker pageWalker2;

    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    EXPECT_EQ(0u, pageWalker2.pages64KB.size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(3u, pageWalker2.entries.size());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForSameAddressTwiceWithDifferentPhysicalAddressAndMemoryBank) {
    const uint64_t gfxAddress = 1ull << (ppgtt->getNumAddressBits() - 9);
    const size_t size = 4096u;

    const uint64_t firstPhysicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_SYSTEM, size, 65536);
    const uint64_t secondPhysicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, size, 65536);
    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_SYSTEM, 0, size}, PageTableWalker::WalkMode::Reserve, nullptr, firstPhysicalAddress);

    PageTableWalker pageWalker2;

    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, size}, PageTableWalker::WalkMode::Reserve, nullptr, secondPhysicalAddress);

    EXPECT_EQ(pageWalker.entries[0].physicalAddress, firstPhysicalAddress);
    EXPECT_EQ(pageWalker.entries[0].isLocalMemory, false);
    EXPECT_EQ(pageWalker.entries[0].memoryBank, MEMORY_BANK_SYSTEM);

    // Second walk should have updated both physical address and memory bank
    EXPECT_EQ(pageWalker2.entries[0].physicalAddress, secondPhysicalAddress);
    EXPECT_EQ(pageWalker2.entries[0].isLocalMemory, true);
    EXPECT_EQ(pageWalker2.entries[0].memoryBank, MEMORY_BANK_0);
}

using PageTableWalkerMultipleBanksTest = PageTableWalkerFixture<PML4, 2>;

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryWithNewPhysicalMemoryThenPageTableIsUpdatedWithPhysicalAddressAndMemoryBank) {
    const uint64_t gfxAddress = ppgtt->getNumAddressBits() == 48
                                    ? (1ull << 39) - sizeof(uint32_t)
                                    : (1ull << 30) - sizeof(uint32_t);
    const size_t size = 2 * 65536u;

    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, size, 65536);
    PageTableWalker pageWalker;

    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    EXPECT_EQ(0u, pageWalker.pages64KB.size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(3u, pageWalker.entries.size());

    EXPECT_EQ(physicalAddress + (gfxAddress & (65536u - 1)), pageWalker.entries[0].physicalAddress);
    EXPECT_EQ(physicalAddress + (65536u - (gfxAddress & (65536u - 1))), pageWalker.entries[1].physicalAddress);

    auto pdp = ppgtt->getChild(0);
    ASSERT_NE(nullptr, pdp);

    const auto pdpIndex = pdp->getIndex(gfxAddress);
    auto pde = pdp->getChild(pdpIndex);
    ASSERT_NE(nullptr, pde);

    const auto pdeIndex = pde->getIndex(gfxAddress);
    auto pte = pde->getChild(pdeIndex);
    ASSERT_NE(nullptr, pte);

    const auto pteIndex = pte->getIndex(gfxAddress);
    auto page = pte->getChild(pteIndex);
    ASSERT_NE(nullptr, page);
    EXPECT_EQ(physicalAddress, page->getPhysicalAddress());
    EXPECT_EQ(MEMORY_BANK_0, page->getMemoryBank());

    const uint64_t physicalAddress2 = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_1, size, 65536);
    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_1, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress2);

    EXPECT_EQ(0u, pageWalker2.pages64KB.size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(3u, pageWalker2.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(physicalAddress2 + (gfxAddress & (65536u - 1)), pageWalker2.entries[0].physicalAddress);

    auto newPage = pte->getChild(pteIndex);
    ASSERT_NE(nullptr, newPage);
    EXPECT_TRUE(newPage->isLocalMemory());
    EXPECT_EQ(physicalAddress2, newPage->getPhysicalAddress());
    EXPECT_EQ(MEMORY_BANK_1, newPage->getMemoryBank());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageThenPageTableWalkerHasCorrectEntriesAtLevel1) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000; // 2MB aligned address
    const size_t size = Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    // For 2MB pages, leafLevel = PageTableLevel::Pde, so pageWalkEntries[PageTableLevel::Pte] should be empty
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());

    EXPECT_EQ(1u, pageWalker.entries.size());
    EXPECT_EQ(size, pageWalker.entries[0].size);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageThenNoPTEIsCreated) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000;
    const size_t size = Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    // Verify hierarchy: PML4 -> PDP -> PDE -> Page2MB (no PTE)
    auto pdp = ppgtt->getChild(ppgtt->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);

    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);

    auto page = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page);

    // The child of PDE should be a Page2MB
    EXPECT_EQ(Page2MB::pageSize2MB, page->getPageSize());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageThenPhysicalAddressIsAlignedTo2MB) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000;
    const size_t size = Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    ASSERT_EQ(1u, pageWalker.entries.size());
    EXPECT_EQ(0u, pageWalker.entries[0].physicalAddress % Page2MB::pageSize2MB);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForMultiple2MBPagesThenCorrectNumberOfEntriesCreated) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000;
    const size_t numEntries = 3;
    const size_t size = numEntries * Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(numEntries, pageWalker.entries.size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    // PDE level should have 3 entries
    EXPECT_EQ(numEntries, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageCrossingPDPBoundaryThenMultiplePDPsUsed) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // Cross 1GB boundary (PDP boundary) with two 2MB pages
    const uint64_t gfxAddress = (1ull << 30) - Page2MB::pageSize2MB;
    const size_t size = 2 * Page2MB::pageSize2MB; // Crosses into second PDP entry

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(2u, pageWalker.entries.size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
}

TEST_F(PageTableWalkerTest, givenCloneModeAndPPGTTWhenWalkingMemoryFor2MBPageThenPageIsClonedWithSamePhysicalAddress) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000;
    const size_t size = Page2MB::pageSize2MB;

    PageTableWalker pageWalkerReserve;
    pageWalkerReserve.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    std::vector<PageInfo> pageInfosForCloning = pageWalkerReserve.entries;

    auto ppgtt2 = std::make_unique<PML4>(*gpu, allocator.get(), defaultMemoryBank);

    PageTableWalker pageWalkerClone;
    pageWalkerClone.walkMemory(ppgtt2.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Clone, &pageInfosForCloning);

    EXPECT_EQ(0u, pageWalkerClone.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(1u, pageWalkerClone.pageWalkEntries[PageTableLevel::Pde].size());
    ASSERT_EQ(1u, pageWalkerClone.entries.size());
    EXPECT_EQ(pageInfosForCloning[0].physicalAddress, pageWalkerClone.entries[0].physicalAddress);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageWithPreReservedPhysicalAddressThenCorrectAddressIsUsed) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000;
    const size_t size = Page2MB::pageSize2MB;
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, Page2MB::pageSize2MB, Page2MB::pageSize2MB);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    ASSERT_EQ(1u, pageWalker.entries.size());
    EXPECT_EQ(physicalAddress, pageWalker.entries[0].physicalAddress);

    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageWithUnalignedAddressThenUses2MBPagesWithOffset) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x210000; // not 2 MB aligned
    const size_t size = Page2MB::pageSize2MB;
    const uint64_t expectedOffset = gfxAddress & (Page2MB::pageSize2MB - 1);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_NE(0u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());

    auto pdp = ppgtt->getChild(ppgtt->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    auto page = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page);
    EXPECT_EQ(Page2MB::pageSize2MB, page->getPageSize());

    // Crosses a 2 MB boundary into the next page
    ASSERT_EQ(2u, pageWalker.entries.size());

    EXPECT_EQ(expectedOffset, pageWalker.entries[0].physicalAddress % Page2MB::pageSize2MB);
    EXPECT_EQ(Page2MB::pageSize2MB - expectedOffset, pageWalker.entries[0].size);

    EXPECT_EQ(0u, pageWalker.entries[1].physicalAddress % Page2MB::pageSize2MB);
    EXPECT_EQ(expectedOffset, pageWalker.entries[1].size);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBAndThen64KBAtDifferentRegionsThenNoConflict) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress2MB = 0x200000;
    const size_t size2MB = Page2MB::pageSize2MB;

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {gfxAddress2MB, nullptr, size2MB, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    const uint64_t gfxAddress64KB = 0x410000;
    const size_t size64KB = 65536;

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress64KB, nullptr, size64KB, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(1u, pageWalker1.entries.size());
    EXPECT_EQ(1u, pageWalker2.entries.size());
    EXPECT_EQ(size2MB, pageWalker1.entries[0].size);
    EXPECT_EQ(size64KB, pageWalker2.entries[0].size);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageThenSubsequent64KBInSameRegionReuses2MBPage) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress1 = 0x8000ffc00000ull; // 2MB aligned
    const size_t size1 = Page2MB::pageSize2MB;

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {gfxAddress1, nullptr, size1, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    // 64KB request within same 2MB region - should reuse existing Page2MB
    const uint64_t gfxAddress2 = 0x8000ffc10000ull;
    const size_t size2 = 65536;

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress2, nullptr, size2, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(1u, pageWalker1.entries.size());
    EXPECT_EQ(1u, pageWalker2.entries.size());

    // Both should map to the same 2MB physical page
    auto phys1Base = pageWalker1.entries[0].physicalAddress & ~(Page2MB::pageSize2MB - 1);
    auto phys2Base = pageWalker2.entries[0].physicalAddress & ~(Page2MB::pageSize2MB - 1);
    EXPECT_EQ(phys1Base, phys2Base);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageWithUnalignedAddressThenSubsequent64KBInSameRegionReuses2MBPage) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // 2MB page at unaligned address
    const uint64_t gfxAddress1 = 0x210000;
    const size_t size1 = 65536;

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {gfxAddress1, nullptr, size1, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(0u, pageWalker1.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(1u, pageWalker1.entries.size());

    // 64 KB request at different offset in same 2 MB region
    const uint64_t gfxAddress2 = 0x220000;
    const size_t size2 = 65536;

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress2, nullptr, size2, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // Reuses existing Page2MB
    EXPECT_EQ(0u, pageWalker2.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(1u, pageWalker2.entries.size());

    // Same 2MB physical page
    auto phys1Base = pageWalker1.entries[0].physicalAddress & ~(Page2MB::pageSize2MB - 1);
    auto phys2Base = pageWalker2.entries[0].physicalAddress & ~(Page2MB::pageSize2MB - 1);
    EXPECT_EQ(phys1Base, phys2Base);

    // Correct offsets within the page
    EXPECT_EQ(0x10000u, pageWalker1.entries[0].physicalAddress % Page2MB::pageSize2MB);
    EXPECT_EQ(0x20000u, pageWalker2.entries[0].physicalAddress % Page2MB::pageSize2MB);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhen64KBFirstThen2MBRequestedInSameRegionThenUsesExistingPTEStructure) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // 64 KB page creates PTE structure at this PDE
    const uint64_t gfxAddress64KB = 0x210000;
    const size_t size64KB = 65536;

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {gfxAddress64KB, nullptr, size64KB, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // 2MB requested in same PDE - must keep existing PTE structure
    const uint64_t gfxAddress2MB = 0x200000;
    const size_t size2MB = Page2MB::pageSize2MB;

    // pageWalker1 was never committed (no writePageWalkEntries), so its nodes have pendingWrite=true.
    // pageWalker2 re-emits the pre-existing PTE: 31 new + 1 pending = 32 total PTE entries.
    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {gfxAddress2MB, nullptr, size2MB, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(32u, pageWalker2.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(32u, pageWalker2.entries.size());

    // PTE structure preserved not replaced by Page2MB
    auto pdp = ppgtt->getChild(ppgtt->getIndex(gfxAddress2MB));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress2MB));
    ASSERT_NE(nullptr, pde);
    auto pte = pde->getChild(pde->getIndex(gfxAddress2MB));
    ASSERT_NE(nullptr, pte);
    EXPECT_NE(Page2MB::pageSize2MB, pte->getPageSize());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhen2MBRequestedButHardwareDoesNotSupportThenFallsBackTo64KB) {
    TEST_REQUIRES(localMemorySupportedInTests);
    if (gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB)) {
        GTEST_SKIP() << "GPU supports 2MB pages, cannot test hardware fallback.";
    }

    const uint64_t gfxAddress = 0x200000; // 2 MB aligned
    const size_t size = Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    // Fallback to 64KB pages
    EXPECT_NE(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());

    // PTE structure, not Page2MB
    auto pdp = ppgtt->getChild(ppgtt->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    auto pte = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pte);
    EXPECT_NE(Page2MB::pageSize2MB, pte->getPageSize());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryFor2MBPageWithPreReservedPhysicalAddressAndUnalignedGfxAddressThenPhysicalAddressAdvancesTo2MBBoundary) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // Unaligned by 0x10000 (split over two 2MB pages)
    const uint64_t gfxAddress = 0x210000;
    const size_t size = Page2MB::pageSize2MB;
    const size_t physicalSize = 2 * Page2MB::pageSize2MB;
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, physicalSize, Page2MB::pageSize2MB);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    ASSERT_EQ(2u, pageWalker.entries.size());
    EXPECT_EQ(0u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());

    const uint64_t expectedOffset = gfxAddress & (Page2MB::pageSize2MB - 1); // 0x10000
    EXPECT_EQ(physicalAddress + expectedOffset, pageWalker.entries[0].physicalAddress);
    EXPECT_EQ(Page2MB::pageSize2MB - expectedOffset, pageWalker.entries[0].size);

    // Second entry starts at next 2 MB boundary
    EXPECT_EQ(physicalAddress + Page2MB::pageSize2MB, pageWalker.entries[1].physicalAddress);
    EXPECT_EQ(expectedOffset, pageWalker.entries[1].size);

    // Both Page2MB nodes must be 2MB-aligned
    auto pdp = ppgtt->getChild(ppgtt->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);

    auto page1 = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page1);
    EXPECT_EQ(0u, page1->getPhysicalAddress() % Page2MB::pageSize2MB);

    auto page2 = pde->getChild(pde->getIndex(gfxAddress + Page2MB::pageSize2MB));
    ASSERT_NE(nullptr, page2);
    EXPECT_EQ(0u, page2->getPhysicalAddress() % Page2MB::pageSize2MB);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhenWalkingMemoryForMultiple2MBPagesWithPreReservedPhysicalAddressThenPhysicalAddressesAreSequential) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    const uint64_t gfxAddress = 0x200000;
    const size_t numPages = 3;
    const size_t size = numPages * Page2MB::pageSize2MB;
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, size, Page2MB::pageSize2MB);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    ASSERT_EQ(numPages, pageWalker.entries.size());

    for (size_t i = 0; i < numPages; i++) {
        EXPECT_EQ(physicalAddress + i * Page2MB::pageSize2MB, pageWalker.entries[i].physicalAddress);
        EXPECT_EQ(Page2MB::pageSize2MB, pageWalker.entries[i].size);
    }
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWithMultipleBanksWhen2MBPageWithUnalignedAddressThenMemoryBankColoringIsCorrect) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_1, Page2MB::pageSize2MB));

    const uint32_t memoryBanks = MEMORY_BANK_0 | MEMORY_BANK_1;
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgttMultiBank(*gpu, &allocator, MEMORY_BANK_0);

    // Unaligned (spans two 2MB pages)
    const uint64_t gfxAddress = 0x210000;
    const size_t size = Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(&ppgttMultiBank, {gfxAddress, nullptr, size, memoryBanks, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    ASSERT_EQ(2u, pageWalker.entries.size());

    EXPECT_EQ(MEMORY_BANK_0, pageWalker.entries[0].memoryBank);
    EXPECT_EQ(MEMORY_BANK_1, pageWalker.entries[1].memoryBank);
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWhen2MBRequestedAndConflictAtOnePDEThenSubsequentPDEsRecover2MBPages) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // Pre-create 64 KB PTE at PDE 1 to cause a conflict
    const uint64_t conflictAddress = 0x210000;
    // setupWalker was never committed, so its nodes have pendingWrite=true.
    // pageWalker re-emits the conflict PDE (PTE64KB) and the pre-existing data PTE.
    PageTableWalker setupWalker;
    setupWalker.walkMemory(ppgtt.get(), {conflictAddress, nullptr, 65536, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // Walk 6 MB with 2 MB pages: PDE 1 conflicts 64KB, PDE 2-3 2MB
    const uint64_t gfxAddress = 0x200000;
    const size_t size = 3 * Page2MB::pageSize2MB;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(34u, pageWalker.entries.size());

    // 32 PTE entries: 31 new + 1 re-emitted pending from setupWalker
    EXPECT_EQ(32u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());

    // PDE entries: 1 for conflict PDE (PTE64KB re-emitted pending) + 2 for Page2MB (PDE 2 and 3)
    EXPECT_EQ(3u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());

    auto pdp = ppgtt->getChild(ppgtt->getIndex(0x400000));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(0x400000));
    ASSERT_NE(nullptr, pde);

    auto page2 = pde->getChild(pde->getIndex(0x400000));
    ASSERT_NE(nullptr, page2);
    EXPECT_EQ(Page2MB::pageSize2MB, page2->getPageSize());

    auto page3 = pde->getChild(pde->getIndex(0x600000));
    ASSERT_NE(nullptr, page3);
    EXPECT_EQ(Page2MB::pageSize2MB, page3->getPageSize());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWithPreReservedPhysicalAddressWhenConflictAtOnePDEThenSubsequentPDEsRecover2MBPagesWithAlignedPhysicalAddress) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // Pre-create 64KB PTE at PDE 1
    const uint64_t conflictAddress = 0x210000;
    PageTableWalker setupWalker;
    setupWalker.walkMemory(ppgtt.get(), {conflictAddress, nullptr, 65536, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // Walk 4 MB: PDE 1 conflicts - 64KB, PDE 2 - 2MB with pre-reserved physical memory
    const uint64_t gfxAddress = 0x200000;
    const size_t size = 2 * Page2MB::pageSize2MB;
    const size_t physicalSize = 2 * Page2MB::pageSize2MB;
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, physicalSize, Page2MB::pageSize2MB);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    EXPECT_EQ(33u, pageWalker.entries.size()); // 32 (PDE 1 - 64KB) + 1 (PDE 2 - 2MB)

    // PDE 2: Page2MB with 2MB-aligned physical address
    auto pdp = ppgtt->getChild(ppgtt->getIndex(0x400000));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(0x400000));
    ASSERT_NE(nullptr, pde);

    auto page2 = pde->getChild(pde->getIndex(0x400000));
    ASSERT_NE(nullptr, page2);
    EXPECT_EQ(Page2MB::pageSize2MB, page2->getPageSize());
    EXPECT_EQ(0u, page2->getPhysicalAddress() % Page2MB::pageSize2MB);

    // Next 2MB boundary after the conflict PDE
    EXPECT_EQ(physicalAddress + Page2MB::pageSize2MB, page2->getPhysicalAddress());
}

TEST_F(PageTableWalkerTest, givenReserveModeAndPPGTTWithPreReservedPhysicalAddressWhenConflictAtOnePDEAndUnalignedGfxAddressThenPhysicalAddressIsRoundedUpTo2MBBoundary) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    // Pre-create 64 KB PTE at PDE 1
    const uint64_t conflictAddress = 0x210000;
    PageTableWalker setupWalker;
    setupWalker.walkMemory(ppgtt.get(), {conflictAddress, nullptr, 65536, MEMORY_BANK_0, 0, 65536}, PageTableWalker::WalkMode::Reserve, nullptr);

    // Unaligned gfxAddress within conflict PDE. After 64KB fallback with partial first page,
    // physicalAddress ends at base+0x1F8000 (not 2MB-aligned), must be round up for PDE 2.
    const uint64_t gfxAddress = 0x208000;
    const size_t size = 2 * Page2MB::pageSize2MB;

    const size_t physicalSize = 3 * Page2MB::pageSize2MB; // accounts for gfxOffset rounding
    const uint64_t physicalAddress = ppgtt->getPhysicalAddressAllocator()->reservePhysicalMemory(MEMORY_BANK_0, physicalSize, Page2MB::pageSize2MB);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, Page2MB::pageSize2MB}, PageTableWalker::WalkMode::Reserve, nullptr, physicalAddress);

    // PDE 1: 32 (64KB, conflict), PDE 2: 1 (2MB), PDE 3: 1 (2MB, partial)
    EXPECT_EQ(34u, pageWalker.entries.size());

    // PDE 2: Page2MB with aligned physical address
    auto pdp = ppgtt->getChild(ppgtt->getIndex(0x400000));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(0x400000));
    ASSERT_NE(nullptr, pde);

    auto page2 = pde->getChild(pde->getIndex(0x400000));
    ASSERT_NE(nullptr, page2);
    EXPECT_EQ(Page2MB::pageSize2MB, page2->getPageSize());
    EXPECT_EQ(0u, page2->getPhysicalAddress() % Page2MB::pageSize2MB);

    // Rounded up from base+0x1F8000 to base+2MB
    EXPECT_EQ(physicalAddress + Page2MB::pageSize2MB, page2->getPhysicalAddress());

    // PDE 3: also 2MB-aligned
    auto page3 = pde->getChild(pde->getIndex(0x600000));
    ASSERT_NE(nullptr, page3);
    EXPECT_EQ(Page2MB::pageSize2MB, page3->getPageSize());
    EXPECT_EQ(0u, page3->getPhysicalAddress() % Page2MB::pageSize2MB);
    EXPECT_EQ(physicalAddress + 2 * Page2MB::pageSize2MB, page3->getPhysicalAddress());
}

using PageTableWalkerTestPml5Ps64 = PageTableWalkerFixture<PML5, 1>;

TEST_F(PageTableWalkerTestPml5Ps64, givenPartialGroupWhenCompletedWithDifferentPermissionBitsThenPS64BitNotSet) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    // Map first 8 pages with local memory
    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 8 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    // Complete the group with system memory - same consecutive PAs, but localMemoryBit differs
    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x18000, nullptr, 8 * 4096, MEMORY_BANK_SYSTEM, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase + 8 * 4096);

    auto &pteEntries = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(8u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_FALSE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenFullGroupWhenPhysAddrIsNotAligned64KBThenPS64BitNotSet) {
    const uint64_t alignedBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);
    const uint64_t physBase = alignedBase + 4096; // guaranteed not 64KB aligned

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                          PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    auto &pteEntries = pageWalker.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_FALSE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenFullGroupWhenGfxAddrIsNotAligned64KBThenPS64BitNotSet) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {0x11000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                          PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    auto &pteEntries = pageWalker.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_FALSE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenAlignedPs64GroupWhenMappedThenPS64BitSetInAllPTEs) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                          PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    auto &pteEntries = pageWalker.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (size_t i = 0; i < pteEntries.size(); i++) {
        EXPECT_TRUE(pteEntries[i].tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenAlignedPs64GroupWhenRemappingUnalignedStartWithNonConsecutivePhysAddrThenPS64BitReset) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    const uint64_t physOther = allocator->reservePhysicalMemory(MEMORY_BANK_0, 4096, 4096);
    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x13000, nullptr, 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physOther);

    auto &pteEntries = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_FALSE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenAlignedPs64GroupWhenRemappingUnalignedEndWithNonConsecutivePhysAddrThenPS64BitReset) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    const uint64_t physOther = allocator->reservePhysicalMemory(MEMORY_BANK_0, 4096, 4096);
    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x1e000, nullptr, 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physOther);

    auto &pteEntries = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_FALSE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPartialPs64GroupWhenCompletedFromUnalignedStartThenPS64BitSet) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 13 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x1d000, nullptr, 3 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase + 13 * 4096);

    auto &pteEntries = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (size_t i = pteEntries.size() - 16; i < pteEntries.size(); i++) {
        EXPECT_TRUE(pteEntries[i].tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPartialPs64GroupWhenCompletedAtUnalignedEndThenPS64BitSet) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x12000, nullptr, 14 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase + 2 * 4096);

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x10000, nullptr, 2 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    auto &pteEntries = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    // Page 0: 1 PTE entry; page 1 completes the group: batch of 16
    EXPECT_EQ(17u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (size_t i = pteEntries.size() - 16; i < pteEntries.size(); i++) {
        EXPECT_TRUE(pteEntries[i].tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenAlignedPs64GroupWhenRemappedToAnotherAlignedPhysAddrThenPS64BitSet) {
    const uint64_t physBase1 = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);
    const uint64_t physBase2 = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase1);

    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    auto &pteEntries1 = pageWalker1.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries1.size());
    for (const auto &entry : pteEntries1) {
        EXPECT_TRUE(entry.tableEntry & ps64Mask);
    }

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase2);

    auto &pteEntries2 = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    // wasPs64Group downgrade emits 16; willComplete re-maps slots 1..14 one by one (14 entries);
    // isPs64GroupComplete re-emits slots 0 and 15 as PS64 (2 entries) = 32 total
    EXPECT_EQ(32u, pteEntries2.size());
    for (size_t i = pteEntries2.size() - 16; i < pteEntries2.size(); i++) {
        EXPECT_TRUE(pteEntries2[i].tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPs64GroupWhenRemappedToSamePhysAddrThenNoPTEEntriesEmitted) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    simulateWritePageWalkEntries(pageWalker1);

    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    // All 16 PTEs already have isPs64=true - isPs64GroupComplete branch skips all of them
    EXPECT_EQ(0u, pageWalker2.pageWalkEntries[PageTableLevel::Pte].size());
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPartialGroupWhenSlot0HasNonConsecutivePhysAddrThenPS64BitNotSet) {
    const uint64_t physBase1 = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);
    const uint64_t physBase2 = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    // Map PTE 0 with physBase1
    PageTableWalker pageWalker1;
    pageWalker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase1);

    // Map PTEs 1-15 starting at physBase2+4096: expectedPhysBase=physBase2 is 64KB-aligned and
    // size is sufficient, but c0->PA=physBase1 != physBase2, so willComplete=false for all slots
    PageTableWalker pageWalker2;
    pageWalker2.walkMemory(ppgtt.get(), {0x11000, nullptr, 15 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr, physBase2 + 4096);

    auto &pteEntries = pageWalker2.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(15u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_FALSE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPs64SupportedWhenMappingWith64KBPageSizeThenPS64BitNotSet) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 65536, MEMORY_BANK_0, 0, 65536},
                          PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    auto &pteEntries = pageWalker.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(1u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    EXPECT_FALSE(pteEntries[0].tableEntry & ps64Mask);
}

TEST_F(PageTableWalkerTest, givenMultiplePagesInSameRegionWhenWalkingThenSharedInteriorNodesEmittedExactlyOnce) {
    const uint64_t gfxAddress = 0x1000;
    const size_t size = 2 * 4096;

    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, size, MEMORY_BANK_0, 0, 4096},
                          PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
}

TEST_F(PageTableWalkerTestPml5Ps64, givenMultiplePagesInSameRegionWhenWalkingWithPs64EnabledThenSharedInteriorNodesEmittedExactlyOnce) {
    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    // Two pages in the same region - PS64 group stays incomplete (only 2 of 16 slots filled)
    const uint64_t gfxAddress = 0x1000;
    PageTableWalker pageWalker;
    pageWalker.walkMemory(ppgtt.get(), {gfxAddress, nullptr, 2 * 4096, MEMORY_BANK_0, 0, 4096},
                          PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pml5].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pml4].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pdp].size());
    EXPECT_EQ(1u, pageWalker.pageWalkEntries[PageTableLevel::Pde].size());
    EXPECT_EQ(2u, pageWalker.pageWalkEntries[PageTableLevel::Pte].size());
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPs64GroupPendingFromPriorWalkWhenSubsequentWalkThenAllGroupEntriesEmitted) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    // Reserve without commit - walker discarded, leaving pendingWrite=true on all nodes
    PageTableWalker reserveWalker;
    reserveWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                             PageTableWalker::WalkMode::Reserve, nullptr, physBase);

    PageTableWalker writeWalker;
    writeWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr);

    auto &pteEntries = writeWalker.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_TRUE(entry.tableEntry & ps64Mask);
    }
    EXPECT_EQ(16u, writeWalker.pendingNodes[PageTableLevel::Pte].size());
}

TEST_F(PageTableWalkerTestPml5Ps64, givenPartialPs64GroupCommittedAt4KBWhenCompletedByReserveWalkThenSubsequentWalkEmitsAllGroupEntriesAsPs64) {
    const uint64_t physBase = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    // Map first 13 pages as committed 4KB entries (isPs64=false, pendingWrite=false after commit)
    PageTableWalker walker13;
    walker13.walkMemory(ppgtt.get(), {0x10000, nullptr, 13 * 4096, MEMORY_BANK_0, 0, 4096},
                        PageTableWalker::WalkMode::Reserve, nullptr, physBase);
    simulateWritePageWalkEntries(walker13);

    // Reserve walk adds the remaining 3 pages, completing the group; walker is discarded
    PageTableWalker reserveWalker;
    reserveWalker.walkMemory(ppgtt.get(), {0x1d000, nullptr, 3 * 4096, MEMORY_BANK_0, 0, 4096},
                             PageTableWalker::WalkMode::Reserve, nullptr, physBase + 13 * 4096);

    // Write walk must emit all 16 PTE entries with the PS64 bit set
    PageTableWalker writeWalker;
    writeWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr);

    auto &pteEntries = writeWalker.pageWalkEntries[PageTableLevel::Pte];
    EXPECT_EQ(16u, pteEntries.size());
    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    for (const auto &entry : pteEntries) {
        EXPECT_TRUE(entry.tableEntry & ps64Mask);
    }
}

TEST_F(PageTableWalkerTestPml5Ps64, givenCommittedPs64GroupWhenDowngradedByDiscardedWalkThenSubsequentWalkEmitsAllPendingEntries) {
    const uint64_t physBase1 = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);
    const uint64_t physBase2 = allocator->reservePhysicalMemory(MEMORY_BANK_0, 65536, 65536);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    // Commit a full PS64 group
    PageTableWalker walker1;
    walker1.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                       PageTableWalker::WalkMode::Reserve, nullptr, physBase1);
    simulateWritePageWalkEntries(walker1);

    // Downgrade walk: remap slot 0 to a non-consecutive address, breaking the group.
    // Walker is discarded without commit - setPs64(false) mutations must remain pending.
    PageTableWalker downgradedWalker;
    downgradedWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 4096, MEMORY_BANK_0, 0, 4096},
                                PageTableWalker::WalkMode::Reserve, nullptr, physBase2);

    // Subsequent committed walk must re-emit exactly 16 entries and track all for cleanup
    PageTableWalker writeWalker;
    writeWalker.walkMemory(ppgtt.get(), {0x10000, nullptr, 16 * 4096, MEMORY_BANK_0, 0, 4096},
                           PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_EQ(16u, writeWalker.pageWalkEntries[PageTableLevel::Pte].size());
    EXPECT_EQ(16u, writeWalker.pendingNodes[PageTableLevel::Pte].size());
}
