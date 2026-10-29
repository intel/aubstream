/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "aubstream/allocation_params.h"
#include "aubstream/hint_values.h"
#include "test_defaults.h"
#include "tests/unit_tests/hardware_context_tests.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "gtest/gtest.h"
#include "tests/unit_tests/page_table_helper.h"
#include "variable_backup.h"

using namespace aub_stream;
using ::testing::_;

TEST(PML5, ctorTest) {
    PhysicalAddressAllocatorSimple allocator;
    PML5 pageTable(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_TRUE(pageTable.isLocalMemory());
    EXPECT_EQ(57, pageTable.getNumAddressBits());
    EXPECT_EQ(5, pageTable.getNumLevels());
}

TEST(PML5, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PML5 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PML5, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    PML5 pageTable(*gpu, &allocator, defaultMemoryBank);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(0u, pageTable.getIndex(1ull << 16));
    EXPECT_EQ(0u, pageTable.getIndex(1ull << 32));
    EXPECT_EQ(1u, pageTable.getIndex(1ull << 48));
    EXPECT_EQ(130u, pageTable.getIndex(0x412345674123 << 9));
    EXPECT_EQ(270u, pageTable.getIndex(0x876543218765 << 9));
    EXPECT_EQ(511u, pageTable.getIndex(0x1ffffffffffffff));
}

TEST(PML5, GivenHBMTableAndSystemLastLevelPageWalkPageTables) {
    PhysicalAddressAllocatorSimple allocator;
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(&ppgtt, {0, nullptr, 4, MEMORY_BANK_SYSTEM, 0, 0x1000}, PageTableWalker::WalkMode::Reserve, nullptr);

    EXPECT_TRUE(ppgtt.isLocalMemory());

    auto pml4 = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pml4);
    EXPECT_TRUE(pml4->isLocalMemory());

    auto pdp = pml4->getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto pte = pde->getChild(0);
    ASSERT_NE(nullptr, pte);
    EXPECT_TRUE(pte->isLocalMemory());

    auto page = pte->getChild(0);
    ASSERT_NE(nullptr, page);
    EXPECT_FALSE(page->isLocalMemory());
}

TEST(PML5, givenSystemMemoryWhenCloningMemoryThenPageWalkEntriesAreWrittenToStreamAndPTEIsNotWrittenWithMemory) {
    MockAubStream stream;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);
    bool isLocalMemoryPage = false;

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel4)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel5)).Times(1);

    EXPECT_CALL(stream, writeDiscontiguousPages(bytes, sizeof(bytes), _, DataTypeHintValues::TraceNotype)).Times(0);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, DataTypeHintValues::TraceNotype)).Times(0);

    std::vector<PageInfo> entries;
    PageInfo info = {0x2003000, sizeof(bytes), isLocalMemoryPage, ppgtt.getMemoryBank()};
    entries.push_back(info);

    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, 65536));
}

using HardwareContextTestPML5 = HardwareContextTest;

TEST_F(HardwareContextTestPML5, givenHardwareContextWhenCallingFreeMemoryThenEntriesAreRemovedFromPageTable) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();
    context.writeMemory2({gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, 0, defaultPageSize});

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);

    context.freeMemory(gfxAddress, sizeof(bytes));
}

TEST_F(HardwareContextTestPML5, givenPs64GroupWhenPartiallyFreedThenPS64BitClearedInRemainingEntries) {
    PhysicalAddressAllocatorSimple allocator(1, aub_stream::GB, true);
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(&ppgtt, {0x10000, nullptr, 5 * Ps64::groupSize, MEMORY_BANK_SYSTEM, 0, Ps64::pageSize}, PageTableWalker::WalkMode::Reserve, nullptr);

    const uint64_t ps64Mask = toBitValue(PpgttEntryBits::ps64Bit);
    EXPECT_TRUE(PageTableHelper::getEntry(&ppgtt, 0x10000) & ps64Mask);

    // Free first 8 pages - breaks 2 from 3 PS64 group
    stream.freeMemory(&ppgtt, 0x10000 + 12 * 4096, 8 * 4096);
    // Free one aligned group of 16 pages
    stream.freeMemory(&ppgtt, 0x10000 + 48 * 4096, 16 * 4096);

    // Freed slots: no mapping
    for (int i = 12; i < 20; i++) {
        EXPECT_EQ(0u, PageTableHelper::getEntry(&ppgtt, 0x10000 + i * 4096) & ps64Mask);
    }
    // Still mapped but PS64 bit must be cleared
    for (int i = 0; i < 12; i++) {
        EXPECT_FALSE(PageTableHelper::getEntry(&ppgtt, 0x10000 + i * 4096) & ps64Mask);
    }
    for (int i = 20; i < 32; i++) {
        EXPECT_FALSE(PageTableHelper::getEntry(&ppgtt, 0x10000 + i * 4096) & ps64Mask);
    }
    // Still mapped but PS64 bit must survive
    for (int i = 32; i < 48; i++) {
        EXPECT_TRUE(PageTableHelper::getEntry(&ppgtt, 0x10000 + i * 4096) & ps64Mask);
    }
    // Freed slots: no mapping
    for (int i = 48; i < 64; i++) {
        EXPECT_EQ(0u, PageTableHelper::getEntry(&ppgtt, 0x10000 + i * 4096) & ps64Mask);
    }
    // Still mapped but PS64 bit must survive
    for (int i = 64; i < 80; i++) {
        EXPECT_TRUE(PageTableHelper::getEntry(&ppgtt, 0x10000 + i * 4096) & ps64Mask);
    }
}

TEST_F(HardwareContextTestPML5, givenPs64GroupWhenFreedPartiallyThenBlockIsReleasedOnlyAfterLastEntryIsFreed) {
    PhysicalAddressAllocatorSimple allocator(1, aub_stream::GB, true);
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(&ppgtt, {0x10000, nullptr, Ps64::groupSize, MEMORY_BANK_SYSTEM, 0, Ps64::pageSize}, PageTableWalker::WalkMode::Reserve, nullptr);
    ASSERT_EQ(1u, pageWalker.pages64KB.size());
    const uint64_t groupBlock = pageWalker.pages64KB[0];

    stream.freeMemory(&ppgtt, 0x10000, 8 * Ps64::pageSize);
    EXPECT_NE(groupBlock, allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, Ps64::groupSize, Ps64::groupSize));

    stream.freeMemory(&ppgtt, 0x10000 + 8 * Ps64::pageSize, 8 * Ps64::pageSize);
    EXPECT_EQ(groupBlock, allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, Ps64::groupSize, Ps64::groupSize));
}

TEST_F(HardwareContextTestPML5, givenPs64GroupWithRemappedPageWhenRemainingPagesAreFreedThenBlockIsReleased) {
    PhysicalAddressAllocatorSimple allocator(1, aub_stream::GB, true);
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->EnablePs64.set(true);

    PageTableWalker pageWalker;
    pageWalker.walkMemory(&ppgtt, {0x10000, nullptr, Ps64::groupSize, MEMORY_BANK_SYSTEM, 0, Ps64::pageSize}, PageTableWalker::WalkMode::Reserve, nullptr);
    ASSERT_EQ(1u, pageWalker.pages64KB.size());
    const uint64_t groupBlock = pageWalker.pages64KB[0];

    const uint64_t remappedAddress = 0x10000 + 3 * Ps64::pageSize;
    const uint64_t physOther = allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, Ps64::pageSize, Ps64::pageSize);
    PageTableWalker remapWalker;
    remapWalker.walkMemory(&ppgtt, {remappedAddress, nullptr, Ps64::pageSize, MEMORY_BANK_SYSTEM, 0, Ps64::pageSize}, PageTableWalker::WalkMode::Reserve, nullptr, physOther);

    // Free every page except the remapped one: it no longer uses the block, so the block must be released
    stream.freeMemory(&ppgtt, 0x10000, 3 * Ps64::pageSize);
    stream.freeMemory(&ppgtt, remappedAddress + Ps64::pageSize, 12 * Ps64::pageSize);
    EXPECT_EQ(physOther, PageTableHelper::getPhysicalAddress(&ppgtt, remappedAddress));
    EXPECT_EQ(groupBlock, allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, Ps64::groupSize, Ps64::groupSize));
}

TEST_F(HardwareContextTestPML5, givenOwnedPageWhenRemappedToCallerMemoryThenOwnedPageIsReleasedAndCallerMemoryIsNotFreedWithMapping) {
    PhysicalAddressAllocatorSimple allocator(1, aub_stream::GB, true);
    PML5 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);
    constexpr size_t pageSize = 4096u;
    constexpr uint64_t gfxAddress = 0x10000;

    // Two pages, so the PTE table survives freeing the remapped one and cannot be recycled by the allocator
    PageTableWalker reserveWalker;
    reserveWalker.walkMemory(&ppgtt, {gfxAddress, nullptr, 2 * pageSize, MEMORY_BANK_SYSTEM, 0, pageSize}, PageTableWalker::WalkMode::Reserve, nullptr);
    const uint64_t ownedPage = PageTableHelper::getPhysicalAddress(&ppgtt, gfxAddress);

    const uint64_t callerPage = allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, pageSize, pageSize);
    PageTableWalker remapWalker;
    remapWalker.walkMemory(&ppgtt, {gfxAddress, nullptr, pageSize, MEMORY_BANK_SYSTEM, 0, pageSize}, PageTableWalker::WalkMode::Reserve, nullptr, callerPage);
    EXPECT_EQ(callerPage, PageTableHelper::getPhysicalAddress(&ppgtt, gfxAddress));
    EXPECT_EQ(ownedPage, allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, pageSize, pageSize));

    stream.freeMemory(&ppgtt, gfxAddress, pageSize);
    EXPECT_NE(callerPage, allocator.reservePhysicalMemory(MEMORY_BANK_SYSTEM, pageSize, pageSize));
}
