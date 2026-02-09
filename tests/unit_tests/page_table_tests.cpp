/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_entry_bits.h"
#include "aub_mem_dump/page_table_walker.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_physical_address_allocator.h"
#include "aubstream/hint_values.h"
#include "test.h"

#include "gtest/gtest.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::Return;

struct PageTableTest : public MockAubStreamFixture, public ::testing::Test {
    uint32_t memoryBank = defaultMemoryBank;

    void SetUp() override {
        MockAubStreamFixture::SetUp();
    }

    void TearDown() override {
        MockAubStreamFixture::TearDown();
    }
};

struct PageTableParamTest : public ::testing::TestWithParam<uint32_t> {
    uint32_t memoryBank = defaultMemoryBank;

    void SetUp() override {
        memoryBank = GetParam();
    }
};

using GGTTTest = PageTableParamTest;
using PageTableMemoryTest = PageTableParamTest;
using PageTableMixedMemoryTest = PageTableParamTest;
using PML4Test = PageTableParamTest;
using PDETest = PageTableParamTest;
using PDPTest = PageTableParamTest;
using PDP4Test = PageTableParamTest;
using PTE4KBTest = PageTableParamTest;
using PTE64KBTest = PageTableParamTest;

static uint32_t nonColoredLocalMemoryBanks[] = {
    MEMORY_BANK_0,
    MEMORY_BANK_1,
    MEMORY_BANK_2,
    MEMORY_BANK_3,
};

static uint32_t allMemoryBanks[] = {
    MEMORY_BANK_SYSTEM,
    MEMORY_BANK_0,
    MEMORY_BANK_1,
    MEMORY_BANK_2,
    MEMORY_BANK_3,
    MEMORY_BANK_0 | MEMORY_BANK_1,
    MEMORY_BANK_0 | MEMORY_BANK_1 | MEMORY_BANK_2,
    MEMORY_BANK_0 | MEMORY_BANK_1 | MEMORY_BANK_3,
    MEMORY_BANK_0 | MEMORY_BANK_2,
    MEMORY_BANK_0 | MEMORY_BANK_2 | MEMORY_BANK_3,
    MEMORY_BANK_0 | MEMORY_BANK_3,
    MEMORY_BANK_1 | MEMORY_BANK_2,
    MEMORY_BANK_1 | MEMORY_BANK_2 | MEMORY_BANK_3,
    MEMORY_BANK_1 | MEMORY_BANK_3,
    MEMORY_BANK_2 | MEMORY_BANK_3,
    MEMORY_BANKS_ALL,
};

TEST(GGTT, ctorTestsWithSystemMemory) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    EXPECT_EQ(0 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_FALSE(pageTable.isLocalMemory());
    EXPECT_EQ(32, pageTable.getNumAddressBits());
    EXPECT_EQ(1, pageTable.getNumLevels());
}

TEST(GGTT, ctorTestsWithBank0) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_EQ(0 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, ctorTestsWithBank1) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_1);

    EXPECT_EQ(1 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, ctorTestsWithBank2) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_2);

    EXPECT_EQ(2 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, ctorTestsWithBank3) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_3);

    EXPECT_EQ(3 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, pageSizeShouldBe4KB) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(4096u, pageTable.getPageSize());
}

TEST(GGTT, getPhysicalAddressAllocator) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(&allocator, pageTable.getPhysicalAddressAllocator());
}

TEST(GGTT, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto address = 0xdeadf00d;
    EXPECT_EQ(0xdeadf, pageTable.getIndex(address));
}

TEST(GGTT, allocateChildReturnsNonNullPointer) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto child = pageTable.allocateChild(*gpu, pageTable.getPageSize(), pageTable.getMemoryBank());
    EXPECT_NE(nullptr, child);
    delete child;
}

TEST_P(GGTTTest, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT pageTable(*gpu, &allocator, memoryBank);
    EXPECT_EQ(0u, pageTable.getPhysicalAddress());
    EXPECT_EQ(memoryBank, pageTable.getMemoryBank());
}

INSTANTIATE_TEST_SUITE_P(GGTT,
                         GGTTTest,
                         ::testing::ValuesIn(nonColoredLocalMemoryBanks));

TEST_P(PML4Test, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
    EXPECT_EQ(48, pageTable.getNumAddressBits());
    EXPECT_EQ(4, pageTable.getNumLevels());
}

INSTANTIATE_TEST_SUITE_P(PML4,
                         PML4Test,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PML4, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PML4, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(130u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(270u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST(PDP4, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PDP4 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PDP4, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    PDP4 pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0u));
    EXPECT_EQ(1u, pageTable.getIndex(0x41234567));
    EXPECT_EQ(2u, pageTable.getIndex(0x87654321));
    EXPECT_EQ(3u, pageTable.getIndex(0xffffffff));
}

TEST(PDP4, allocateChildReturnsNonNullPointer) {
    PhysicalAddressAllocatorSimple allocator;
    PDP4 pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto child = pageTable.allocateChild(*gpu, 4096, pageTable.getMemoryBank());
    EXPECT_NE(nullptr, child);
    delete child;
}

TEST_P(PDP4Test, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    PDP4 pageTable(*gpu, &allocator, memoryBank);
    EXPECT_EQ(0u, pageTable.getPhysicalAddress());
    EXPECT_EQ(32, pageTable.getNumAddressBits());
    EXPECT_EQ(3, pageTable.getNumLevels());
}

INSTANTIATE_TEST_SUITE_P(PDP4,
                         PDP4Test,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PDP, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PDP pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST_P(PDPTest, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    PDP pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PDP,
                         PDPTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PDE, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PDE, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(43u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(25u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PDETest, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PDE,
                         PDETest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PTE4KB, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PTE4KB pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PTE4KB, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    PTE4KB pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(116u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(24u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PTE4KBTest, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    PTE4KB pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PTE4KB,
                         PTE4KBTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PTE64KB, getEntryValue) {
    PhysicalAddressAllocatorSimple allocator;
    PTE64KB pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::intermediatePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PTE64KB, getIndex) {
    PhysicalAddressAllocatorSimple allocator;
    PTE64KB pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0x00, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(0x07, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(0x01, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(0x1f, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PTE64KBTest, ctor) {
    PhysicalAddressAllocatorSimple allocator;
    PTE64KB pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PTE64KB,
                         PTE64KBTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST_P(PageTableMemoryTest, ctorWithPhysicalAddress) {
    uint64_t physicalAddress = 0x20000;
    PageTableMemory pageTable(*gpu, physicalAddress, memoryBank);

    EXPECT_EQ(physicalAddress, pageTable.getPhysicalAddress());
    EXPECT_EQ(memoryBank, pageTable.getMemoryBank());
}

TEST(PageTableMemory, getEntryValueSysMem) {
    uint64_t physicalAddress = 0x20000;
    PageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_SYSTEM);

    auto expectedEntryValue = physicalAddress;
    expectedEntryValue |= toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    expectedEntryValue |= gpu->getPPGTTExtraEntryBits({});
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PageTableMemory, getEntryValueLocalMem) {
    uint64_t physicalAddress = 0x20000;
    PageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_0);

    auto expectedEntryValue = physicalAddress;
    expectedEntryValue |= toBitValue(PpgttEntryBits::localMemoryBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    expectedEntryValue |= gpu->getPPGTTExtraEntryBits({});
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

INSTANTIATE_TEST_SUITE_P(PageTableMemory,
                         PageTableMemoryTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST_F(PageTableTest, GivenHBMTableAndSystemLastLevelPageVerifyMemoryLocation) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, defaultPageSize});

    EXPECT_TRUE(ppgtt.isLocalMemory());

    auto pdp = ppgtt.getChild(0);
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

TEST_F(PageTableTest, givenMemoryPropertyWhenWriteCalledThenApplyPageProperties) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    AllocationParams params(0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, defaultPageSize);
    params.additionalParams.compressionEnabled = true;

    stream.writeMemory(&ppgtt, params);

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_FALSE(pdp->peekAllocationParams().compressionEnabled);
    EXPECT_FALSE(pdp->peekAllocationParams().uncached);

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_FALSE(pde->peekAllocationParams().compressionEnabled);
    EXPECT_FALSE(pde->peekAllocationParams().uncached);

    auto pte = pde->getChild(0);
    ASSERT_NE(nullptr, pte);
    EXPECT_FALSE(pte->peekAllocationParams().compressionEnabled);
    EXPECT_FALSE(pte->peekAllocationParams().uncached);

    auto page = pte->getChild(0);
    ASSERT_NE(nullptr, page);
    EXPECT_TRUE(page->peekAllocationParams().compressionEnabled);
    EXPECT_FALSE(page->peekAllocationParams().uncached);
}

TEST_F(PageTableTest, Write4KBFollowedBy64KBPageSizeShouldNotChange) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    size_t size1 = 0x2000;
    size_t size2 = 0xc0;
    auto data1 = new uint8_t[size1];
    auto data2 = new uint8_t[size2];
    memset(data1, 0x1, size1);
    memset(data2, 0x2, size2);

    uint64_t address = 0x0;
    size_t pageSize1 = 0x1000;
    size_t pageSize2 = 0x10000;

    stream.writeMemory(&ppgtt, {address, data1, size1, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize1});
    stream.writeMemory(&ppgtt, {address, data2, size2, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize2});

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto pte = static_cast<PTE *>(pde->getChild(0));
    ASSERT_NE(nullptr, pte);
    EXPECT_EQ(pageSize1, pte->getPageSize());
}

TEST_F(PageTableTest, Write64KBFollowedBy4KBPageSizeShouldNotChange) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    size_t size1 = 0x2000;
    size_t size2 = 0xc0;
    auto data1 = new uint8_t[size1];
    auto data2 = new uint8_t[size2];
    memset(data1, 0x1, size1);
    memset(data2, 0x2, size2);

    uint64_t address = 0x0;
    size_t pageSize1 = 0x10000;
    size_t pageSize2 = 0x1000;

    stream.writeMemory(&ppgtt, {address, data1, size1, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize1});
    stream.writeMemory(&ppgtt, {address, data2, size2, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, pageSize2});

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto pte = static_cast<PTE *>(pde->getChild(0));
    ASSERT_NE(nullptr, pte);
    EXPECT_EQ(pageSize1, pte->getPageSize());
}

TEST_F(PageTableTest, GivenHBMPML4TableAndLastLevelPageSystemPageVerifyNewPageTableHintsAreUsed) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});
}

TEST_F(PageTableTest, GivenHBMPML4TableAndSystemLastLevel64KBPageVerifyReserveContiguousPagesIsCalledForWriteMemory) {
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, reserveContiguousPages(_)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});
}

TEST_F(PageTableTest, GivenHBMPML4TableAndSystemLastLevel64KBPageVerifyReserveContiguousPagesIsCalledForCloneMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x5009000;
    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, reserveContiguousPages(_)).Times(0);

    std::vector<PageInfo> entries;
    PageInfo info = {0x2003000, sizeof(bytes), ppgtt.getMemoryBank() != MEMORY_BANK_SYSTEM, ppgtt.getMemoryBank()};
    entries.push_back(info);

    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, 65536));
}

TEST_F(PageTableTest, GivenHBMGGTTAubStreamWriteMemoryVerifyChildPageAttributes) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ggtt, 0, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 4096);

    auto child = ggtt.getChild(0);
    ASSERT_NE(nullptr, child);
    EXPECT_NE(0u, child->getPhysicalAddress());
    EXPECT_EQ(ggtt.getMemoryBank(), child->getMemoryBank());
    EXPECT_TRUE(child->isLocalMemory());
}

TEST_F(PageTableTest, GivenSystemmemGGTTAubStreamWriteMemoryVerifyChildPageAttributes) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ggtt, 0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 4096);

    auto child = ggtt.getChild(0);
    ASSERT_NE(nullptr, child);
    EXPECT_NE(0u, child->getPhysicalAddress());
    EXPECT_EQ(ggtt.getMemoryBank(), child->getMemoryBank());
    EXPECT_FALSE(child->isLocalMemory());
}

TEST_F(PageTableTest, GivenHBMGGTTExpectMemoryVerifyExpectMemoryTableGetsCalled) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, expectMemoryTable(_, _, _, _)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ggtt, 0, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 4096);
    stream.expectMemory(&ggtt, 0, &data, sizeof(data), 0);
}

TEST(PageTable, unusedVirtualMethods) {
    auto physicalAddress = 0xf00;
    PageTable pageTable(*gpu, physicalAddress, MEMORY_BANK_SYSTEM);

    // These unused virtual methods exist only to make a common recursive structure between PageTable and PageTableMemory.
    // If the object hierarchy changes, they might be eliminated.
    EXPECT_EQ(nullptr, pageTable.allocateChild(*gpu, 4096, MEMORY_BANK_SYSTEM));
    EXPECT_EQ(0, pageTable.getIndex(physicalAddress));
}

TEST(PageTable, PageTableStorageResizedOnSetChildCallInsteadOfConstructor) {
    struct MockPageTable : PageTable {
        using PageTable::PageTable;
        using PageTable::table;
    };

    PhysicalAddressAllocatorSimple allocator;

    MockPageTable pageTable(*gpu, &allocator, 4096u, 512u, MEMORY_BANK_SYSTEM);
    EXPECT_EQ(0u, pageTable.table.size());
    auto child = pageTable.getChild(2u);
    EXPECT_EQ(nullptr, child);
    // now set child to resize the storage.
    pageTable.setChild(2u, nullptr);
    EXPECT_EQ(3u, pageTable.table.size());
}

TEST(PageTable, whenPageTableIsDeletedThenFreePhysicalMemory) {
    const auto allocatorSize = 1 * GB;
    MockPhysicalAddressAllocatorSimple allocator{1, allocatorSize, false};
    auto pMockAllocator = static_cast<MockSimpleAllocator<uint64_t> *>(&allocator.mainAllocator);
    {
        PageTable pageTable(*gpu, &allocator, 0x100, 0, MEMORY_BANK_SYSTEM);
        EXPECT_EQ(pMockAllocator->usedAllocationsMap.size(), 1);
        EXPECT_EQ(pMockAllocator->freeAllocationsMap.size(), 0);
    }
    EXPECT_EQ(pMockAllocator->usedAllocationsMap.size(), 0);
    EXPECT_EQ(pMockAllocator->freeAllocationsMap.size(), 1);
}

TEST(PageTable, getPageSizeReturnsZeroForNonLeafNodes) {
    PhysicalAddressAllocatorSimple allocator;

    // Non-leaf nodes - PML4, PDP, PDE
    PML4 pml4(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pml4.getPageSize());

    PDP pdp(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pdp.getPageSize());

    PDE pde(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pde.getPageSize());
}

using Page2MBTest = PageTableParamTest;

TEST(Page2MB, pageSizeShouldBe2MB) {
    PhysicalAddressAllocatorSimple allocator;
    Page2MB page(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(Page2MB::pageSize2MB, page.getPageSize());
    EXPECT_EQ(2u * 1024u * 1024u, page.getPageSize());
}

TEST(Page2MB, physicalAddressShouldBeAlignedTo2MB) {
    PhysicalAddressAllocatorSimple allocator;
    Page2MB page(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_NE(0u, page.getPhysicalAddress());
    EXPECT_EQ(0u, page.getPhysicalAddress() % Page2MB::pageSize2MB);
}

TEST(Page2MB, ctorWithPhysicalAddress) {
    uint64_t physicalAddress = 0x200000; // 2MB aligned
    Page2MB page(*gpu, physicalAddress, MEMORY_BANK_0);
    EXPECT_EQ(physicalAddress, page.getPhysicalAddress());
    EXPECT_EQ(MEMORY_BANK_0, page.getMemoryBank());
}

TEST(Page2MB, getEntryValueLocalMemory) {
    PhysicalAddressAllocatorSimple allocator;
    Page2MB page(*gpu, &allocator, MEMORY_BANK_0);

    auto expectedBits = toBitValue(PpgttEntryBits::largePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit, PpgttEntryBits::localMemoryBit);
    expectedBits |= gpu->getPPGTTExtraEntryBits({});
    auto expectedEntryValue = page.getPhysicalAddress() | expectedBits;
    EXPECT_EQ(expectedEntryValue, page.getEntryValue());
}

TEST(Page2MB, getEntryValueSystemMemory) {
    PhysicalAddressAllocatorSimple allocator;
    Page2MB page(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    auto expectedBits = toBitValue(PpgttEntryBits::largePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    expectedBits |= gpu->getPPGTTExtraEntryBits({});
    auto expectedEntryValue = page.getPhysicalAddress() | expectedBits;
    EXPECT_EQ(expectedEntryValue, page.getEntryValue());
}

TEST(Page2MB, isLocalMemory) {
    PhysicalAddressAllocatorSimple allocator;
    Page2MB pageLocal(*gpu, &allocator, MEMORY_BANK_0);
    Page2MB pageSystem(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    EXPECT_TRUE(pageLocal.isLocalMemory());
    EXPECT_FALSE(pageSystem.isLocalMemory());
}

TEST_P(Page2MBTest, ctorWithAllocator) {
    PhysicalAddressAllocatorSimple allocator;
    Page2MB page(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, page.getPhysicalAddress());
    EXPECT_EQ(memoryBank, page.getMemoryBank());
    EXPECT_EQ(Page2MB::pageSize2MB, page.getPageSize());
}

INSTANTIATE_TEST_SUITE_P(Page2MB,
                         Page2MBTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PDE, allocateChildReturns2MBPageWhenPageSize2MB) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pde(*gpu, &allocator, MEMORY_BANK_0);
    std::unique_ptr<PageTable> child(pde.allocateChild(*gpu, Page2MB::pageSize2MB, pde.getMemoryBank()));
    ASSERT_NE(nullptr, child);
    EXPECT_EQ(Page2MB::pageSize2MB, child->getPageSize());
}

TEST(PDE, allocateChildReturns4KBPTEWhenPageSize4KB) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pde(*gpu, &allocator, MEMORY_BANK_0);
    std::unique_ptr<PageTable> child(pde.allocateChild(*gpu, 4096, pde.getMemoryBank()));
    ASSERT_NE(nullptr, child);
    EXPECT_EQ(4096u, child->getPageSize());
}

TEST(PDE, allocateChildReturns64KBPTEWhenPageSize64KB) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pde(*gpu, &allocator, MEMORY_BANK_0);
    std::unique_ptr<PageTable> child(pde.allocateChild(*gpu, 65536, pde.getMemoryBank()));
    ASSERT_NE(nullptr, child);
    EXPECT_EQ(65536u, child->getPageSize());
}

TEST_F(PageTableTest, GivenPPGTTWhenWriteMemoryWith2MBPagesThenCorrectHierarchyIsCreated) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress = 0x200000; // 2MB aligned

    stream.writeMemory(&ppgtt, {gfxAddress, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    // Verify hierarchy: PML4 -> PDP -> PDE -> Page2MB (no PTE)
    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    EXPECT_TRUE(pdp->isLocalMemory());

    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    EXPECT_TRUE(pde->isLocalMemory());

    auto page = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page);
    EXPECT_EQ(Page2MB::pageSize2MB, page->getPageSize());
    EXPECT_TRUE(page->isLocalMemory());
}

TEST_F(PageTableTest, GivenPPGTTWith2MBPageWhenFreeMemoryThenPageIsRemoved) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress1 = 0x200000; // First 2MB page
    uint64_t gfxAddress2 = 0x400000; // Second 2MB page (in same PDE)

    // Write two 2MB pages so PDE is not empty after freeing one
    stream.writeMemory(&ppgtt, {gfxAddress1, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});
    stream.writeMemory(&ppgtt, {gfxAddress2, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    // Verify pages exist
    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress1));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress1));
    ASSERT_NE(nullptr, pde);
    auto page1 = pde->getChild(pde->getIndex(gfxAddress1));
    auto page2 = pde->getChild(pde->getIndex(gfxAddress2));
    ASSERT_NE(nullptr, page1);
    ASSERT_NE(nullptr, page2);

    // Free the first page
    stream.freeMemory(&ppgtt, gfxAddress1, Page2MB::pageSize2MB);

    // Verify first page is removed but second remains
    page1 = pde->getChild(pde->getIndex(gfxAddress1));
    page2 = pde->getChild(pde->getIndex(gfxAddress2));
    EXPECT_EQ(nullptr, page1);
    EXPECT_NE(nullptr, page2);
}

TEST_F(PageTableTest, GivenPPGTTWith2MBPageWhenFreeMemoryThenEmptyPDEIsAlsoRemoved) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress = 0x200000;

    stream.writeMemory(&ppgtt, {gfxAddress, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);

    stream.freeMemory(&ppgtt, gfxAddress, Page2MB::pageSize2MB);

    // After freeing the only 2MB page, the PDE should also be removed
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    EXPECT_EQ(nullptr, pde);
}

TEST_F(PageTableTest, GivenPPGTTWithMultiple2MBPagesWhenFreeOneThenOthersRemain) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress1 = 0x200000; // First 2MB page
    uint64_t gfxAddress2 = 0x400000; // Second 2MB page (same PDE)

    stream.writeMemory(&ppgtt, {gfxAddress1, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});
    stream.writeMemory(&ppgtt, {gfxAddress2, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress1));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress1));
    ASSERT_NE(nullptr, pde);

    // Confirm both pages exist
    auto page1 = pde->getChild(pde->getIndex(gfxAddress1));
    auto page2 = pde->getChild(pde->getIndex(gfxAddress2));
    ASSERT_NE(nullptr, page1);
    ASSERT_NE(nullptr, page2);

    // Free first page
    stream.freeMemory(&ppgtt, gfxAddress1, Page2MB::pageSize2MB);

    // First page should be removed, second should remain
    page1 = pde->getChild(pde->getIndex(gfxAddress1));
    page2 = pde->getChild(pde->getIndex(gfxAddress2));
    EXPECT_EQ(nullptr, page1);
    EXPECT_NE(nullptr, page2);

    // PDE should still exist
    pde = pdp->getChild(pdp->getIndex(gfxAddress1));
    EXPECT_NE(nullptr, pde);
}

TEST_F(PageTableTest, Write2MBFollowedBy4KBAtSameAddressShouldNotChangePageSize) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress = 0x200000;

    // First write - with 2MB page
    stream.writeMemory(&ppgtt, {gfxAddress, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    // Second write - with 4KB page at same address
    stream.writeMemory(&ppgtt, {gfxAddress, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 4096});

    auto pdp = ppgtt.getChild(ppgtt.getIndex(gfxAddress));
    ASSERT_NE(nullptr, pdp);
    auto pde = pdp->getChild(pdp->getIndex(gfxAddress));
    ASSERT_NE(nullptr, pde);
    auto page = pde->getChild(pde->getIndex(gfxAddress));
    ASSERT_NE(nullptr, page);

    // Page should still be 2MB
    EXPECT_EQ(Page2MB::pageSize2MB, page->getPageSize());
}

TEST_F(PageTableTest, GivenPPGTTWhenWriteMemoryWith2MBPagesThenCorrectHintsAreUsed) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    // 2MB pages use all 4 levels: PML4 -> PDP -> PDE -> Page2MB (no PTE)
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0x200000, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});
}

TEST(PDE, allocateChildReturns2MBPageWhenPageSize2MBWithAdditionalParams) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pde(*gpu, &allocator, MEMORY_BANK_0);

    AllocationParams::AdditionalParams additionalParams = {};
    additionalParams.compressionEnabled = true;

    std::unique_ptr<PageTable> child(pde.allocateChild(*gpu, Page2MB::pageSize2MB, MEMORY_BANK_0, additionalParams));
    ASSERT_NE(nullptr, child);
    EXPECT_EQ(Page2MB::pageSize2MB, child->getPageSize());
    EXPECT_TRUE(child->peekAllocationParams().compressionEnabled);
}

TEST(PDE, allocateChildReturns2MBPageWhenPageSize2MBWithAdditionalParamsAndPhysicalAddress) {
    PhysicalAddressAllocatorSimple allocator;
    PDE pde(*gpu, &allocator, MEMORY_BANK_0);

    AllocationParams::AdditionalParams additionalParams = {};
    additionalParams.compressionEnabled = true;
    uint64_t physicalAddress = 0x400000; // 2MB aligned

    std::unique_ptr<PageTable> child(pde.allocateChild(*gpu, Page2MB::pageSize2MB, MEMORY_BANK_0, additionalParams, physicalAddress));
    ASSERT_NE(nullptr, child);
    EXPECT_EQ(Page2MB::pageSize2MB, child->getPageSize());
    EXPECT_EQ(physicalAddress, child->getPhysicalAddress());
    EXPECT_TRUE(child->peekAllocationParams().compressionEnabled);
}

TEST_F(PageTableTest, GivenPPGTTWith2MBPageWhenFreeMemoryAndPDEBecomesEmptyThenPDPLevelEntryIsWritten) {
    TEST_REQUIRES(localMemorySupportedInTests);
    TEST_REQUIRES(gpu->isMemorySupported(MEMORY_BANK_0, Page2MB::pageSize2MB));

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    uint32_t data = 0xabcdabcd;
    uint64_t gfxAddress = 0x200000;

    stream.writeMemory(&ppgtt, {gfxAddress, &data, sizeof(data), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, Page2MB::pageSize2MB});

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);

    stream.freeMemory(&ppgtt, gfxAddress, Page2MB::pageSize2MB);
}