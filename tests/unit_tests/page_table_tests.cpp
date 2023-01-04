/*
 * Copyright (C) 2022-2023 Intel Corporation
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
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    EXPECT_EQ(0 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_FALSE(pageTable.isLocalMemory());
    EXPECT_EQ(32, pageTable.getNumAddressBits());
    EXPECT_EQ(1, pageTable.getNumLevels());
}

TEST(GGTT, ctorTestsWithBank0) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_EQ(0 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, ctorTestsWithBank1) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_1);

    EXPECT_EQ(1 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, ctorTestsWithBank2) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_2);

    EXPECT_EQ(2 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, ctorTestsWithBank3) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_3);

    EXPECT_EQ(3 * 0x1000000, pageTable.getEntryOffset());
    EXPECT_TRUE(pageTable.isLocalMemory());
}

TEST(GGTT, pageSizeShouldBe4KB) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(4096u, pageTable.getPageSize());
}

TEST(GGTT, getPhysicalAddressAllocator) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(&allocator, pageTable.getPhysicalAddressAllocator());
}

TEST(GGTT, getIndex) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto address = 0xdeadf00d;
    EXPECT_EQ(0xdeadf, pageTable.getIndex(address));
}

TEST(GGTT, allocateChildReturnsNonNullPointer) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto child = pageTable.allocateChild(*gpu, pageTable.getPageSize(), pageTable.getMemoryBank());
    EXPECT_NE(nullptr, child);
    delete child;
}

TEST_P(GGTTTest, ctor) {
    PhysicalAddressAllocator allocator;
    GGTT pageTable(*gpu, &allocator, memoryBank);
    EXPECT_EQ(0u, pageTable.getPhysicalAddress());
    EXPECT_EQ(memoryBank, pageTable.getMemoryBank());
}

INSTANTIATE_TEST_SUITE_P(GGTT,
                         GGTTTest,
                         ::testing::ValuesIn(nonColoredLocalMemoryBanks));

TEST_P(PML4Test, ctor) {
    PhysicalAddressAllocator allocator;
    PML4 pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
    EXPECT_EQ(48, pageTable.getNumAddressBits());
    EXPECT_EQ(4, pageTable.getNumLevels());
}

INSTANTIATE_TEST_SUITE_P(PML4,
                         PML4Test,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PML4, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PML4, getIndex) {
    PhysicalAddressAllocator allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(130u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(270u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST(PDP4, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PDP4 pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PDP4, getIndex) {
    PhysicalAddressAllocator allocator;
    PDP4 pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0u));
    EXPECT_EQ(1u, pageTable.getIndex(0x41234567));
    EXPECT_EQ(2u, pageTable.getIndex(0x87654321));
    EXPECT_EQ(3u, pageTable.getIndex(0xffffffff));
}

TEST(PDP4, allocateChildReturnsNonNullPointer) {
    PhysicalAddressAllocator allocator;
    PDP4 pageTable(*gpu, &allocator, MEMORY_BANK_0);
    auto child = pageTable.allocateChild(*gpu, 4096, pageTable.getMemoryBank());
    EXPECT_NE(nullptr, child);
    delete child;
}

TEST_P(PDP4Test, ctor) {
    PhysicalAddressAllocator allocator;
    PDP4 pageTable(*gpu, &allocator, memoryBank);
    EXPECT_EQ(0u, pageTable.getPhysicalAddress());
    EXPECT_EQ(32, pageTable.getNumAddressBits());
    EXPECT_EQ(3, pageTable.getNumLevels());
}

INSTANTIATE_TEST_SUITE_P(PDP4,
                         PDP4Test,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PDP, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PDP pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST_P(PDPTest, ctor) {
    PhysicalAddressAllocator allocator;
    PDP pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PDP,
                         PDPTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PDE, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PDE pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PDE, getIndex) {
    PhysicalAddressAllocator allocator;
    PDE pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(43u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(25u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PDETest, ctor) {
    PhysicalAddressAllocator allocator;
    PDE pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PDE,
                         PDETest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PTE4KB, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PTE4KB pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PTE4KB, getIndex) {
    PhysicalAddressAllocator allocator;
    PTE4KB pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0u, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(116u, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(24u, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(511u, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PTE4KBTest, ctor) {
    PhysicalAddressAllocator allocator;
    PTE4KB pageTable(*gpu, &allocator, memoryBank);
    EXPECT_NE(0u, pageTable.getPhysicalAddress());
}

INSTANTIATE_TEST_SUITE_P(PTE4KB,
                         PTE4KBTest,
                         ::testing::ValuesIn(allMemoryBanks));

TEST(PTE64KB, getEntryValue) {
    PhysicalAddressAllocator allocator;
    PTE64KB pageTable(*gpu, &allocator, defaultMemoryBank);

    auto expectedEntryValue = pageTable.getPhysicalAddress() | toBitValue(PpgttEntryBits::intermediatePageSizeBit, PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);
    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}

TEST(PTE64KB, getIndex) {
    PhysicalAddressAllocator allocator;
    PTE64KB pageTable(*gpu, &allocator, MEMORY_BANK_0);
    EXPECT_EQ(0x00, pageTable.getIndex(0x000000000000));
    EXPECT_EQ(0x07, pageTable.getIndex(0x412345674123));
    EXPECT_EQ(0x01, pageTable.getIndex(0x876543218765));
    EXPECT_EQ(0x1f, pageTable.getIndex(0xffffffffffff));
}

TEST_P(PTE64KBTest, ctor) {
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});
}

TEST_F(PageTableTest, GivenHBMPML4TableAndSystemLastLevel64KBPageVerifyReserveContiguousPagesIsCalledForWriteMemory) {
    PhysicalAddressAllocator allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, reserveContiguousPages(_)).Times(1);

    uint32_t data = 0xabcdabcd;
    stream.writeMemory(&ppgtt, {0, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 65536});
}

TEST_F(PageTableTest, GivenHBMPML4TableAndSystemLastLevel64KBPageVerifyReserveContiguousPagesIsCalledForCloneMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x5009000;
    PhysicalAddressAllocator allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);

    EXPECT_CALL(stream, reserveContiguousPages(_)).Times(0);

    std::vector<PageInfo> entries;
    PageInfo info = {0x2003000, sizeof(bytes), ppgtt.getMemoryBank() != MEMORY_BANK_SYSTEM, ppgtt.getMemoryBank()};
    entries.push_back(info);

    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, 65536));
}

TEST_F(PageTableTest, GivenHBMGGTTAubStreamWriteMemoryVerifyChildPageAttributes) {
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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
    PhysicalAddressAllocator allocator;
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

    PhysicalAddressAllocator allocator;

    MockPageTable pageTable(*gpu, &allocator, 4096u, 512u, MEMORY_BANK_SYSTEM);
    EXPECT_EQ(0u, pageTable.table.size());
    auto child = pageTable.getChild(2u);
    EXPECT_EQ(nullptr, child);
    // now set child to resize the storage.
    pageTable.setChild(2u, nullptr);
    EXPECT_EQ(3u, pageTable.table.size());
}
