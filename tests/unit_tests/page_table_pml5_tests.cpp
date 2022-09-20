/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "headers/allocation_params.h"
#include "test_defaults.h"
#include "tests/unit_tests/hardware_context_tests.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "gtest/gtest.h"

using namespace aub_stream;
using ::testing::_;

TEST(PML5, ctorTest) {
    PhysicalAddressAllocator allocator;
    PML5 pageTable(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_TRUE(pageTable.isLocalMemory());
    EXPECT_EQ(57, pageTable.getNumAddressBits());
    EXPECT_EQ(5, pageTable.getNumLevels());
}

TEST(PML5, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PML5 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PML5, getIndex) {
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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

    PhysicalAddressAllocator allocator;
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

    auto &csHelper = getCommandStreamerHelper(gpu->productFamily, defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();
    context.writeMemory2({gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, 0, defaultPageSize});

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceNonlocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);

    context.freeMemory(gfxAddress, sizeof(bytes));
}
