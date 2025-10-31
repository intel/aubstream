/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aubstream/allocation_params.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "tests/unit_tests/page_table_helper.h"
#include "gtest/gtest.h"
#include "tests/variable_backup.h"
#include "aubstream/hint_values.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;
namespace mock_os_calls {
extern std::unordered_map<std::string, std::string> environmentStrings;
}

struct AubStreamTest : public MockAubStreamFixture, public ::testing::Test {
    void SetUp() override {
        MockAubStreamFixture::SetUp();
    }

    void TearDown() override {
        MockAubStreamFixture::TearDown();
    }
};

TEST_F(AubStreamTest, multipleWriteMemoryShouldKeepSamePhysicalAddress) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0xbadddadcu;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, defaultMemoryBank);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    EXPECT_NE(0u, ppgtt1.getPhysicalAddress());

    auto pdpe1 = ppgtt1.getChild(ppgtt1.getIndex(gfxAddress));
    EXPECT_NE(0u, pdpe1->getPhysicalAddress());

    auto pde1 = pdpe1->getChild(pdpe1->getIndex(gfxAddress));
    EXPECT_NE(0u, pde1->getPhysicalAddress());

    auto pte1 = pde1->getChild(pde1->getIndex(gfxAddress));
    EXPECT_NE(0u, pte1->getPhysicalAddress());

    auto page1 = pte1->getChild(pte1->getIndex(gfxAddress));
    EXPECT_NE(0u, page1->getPhysicalAddress());

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});
    auto pdpe2 = ppgtt1.getChild(ppgtt1.getIndex(gfxAddress));
    EXPECT_EQ(pdpe1->getPhysicalAddress(), pdpe2->getPhysicalAddress());

    auto pde2 = pdpe2->getChild(pdpe2->getIndex(gfxAddress));
    EXPECT_EQ(pde1->getPhysicalAddress(), pde2->getPhysicalAddress());

    auto pte2 = pde2->getChild(pde2->getIndex(gfxAddress));
    EXPECT_EQ(pte1->getPhysicalAddress(), pte2->getPhysicalAddress());

    auto page2 = pte2->getChild(pte2->getIndex(gfxAddress));
    EXPECT_EQ(page1->getPhysicalAddress(), page2->getPhysicalAddress());
}

TEST_F(AubStreamTest, expectMemoryShouldntAlterPageTable) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0xbadddadcu;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, defaultMemoryBank);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    EXPECT_NE(0u, ppgtt1.getPhysicalAddress());

    auto pdpe1 = ppgtt1.getChild(ppgtt1.getIndex(gfxAddress));
    EXPECT_NE(0u, pdpe1->getPhysicalAddress());

    auto pde1 = pdpe1->getChild(pdpe1->getIndex(gfxAddress));
    EXPECT_NE(0u, pde1->getPhysicalAddress());

    auto pte1 = pde1->getChild(pde1->getIndex(gfxAddress));
    EXPECT_NE(0u, pte1->getPhysicalAddress());

    auto page1 = pte1->getChild(pte1->getIndex(gfxAddress));
    EXPECT_NE(0u, page1->getPhysicalAddress());

    stream.expectMemory(&ppgtt1, gfxAddress, bytes, sizeof(bytes), 0);
    auto pdpe2 = ppgtt1.getChild(ppgtt1.getIndex(gfxAddress));
    EXPECT_EQ(pdpe1->getPhysicalAddress(), pdpe2->getPhysicalAddress());

    auto pde2 = pdpe2->getChild(pdpe2->getIndex(gfxAddress));
    EXPECT_EQ(pde1->getPhysicalAddress(), pde2->getPhysicalAddress());

    auto pte2 = pde2->getChild(pde2->getIndex(gfxAddress));
    EXPECT_EQ(pte1->getPhysicalAddress(), pte2->getPhysicalAddress());

    auto page2 = pte2->getChild(pte2->getIndex(gfxAddress));
    EXPECT_EQ(page1->getPhysicalAddress(), page2->getPhysicalAddress());
}

TEST_F(AubStreamTest, writeMemoryAndClonePageTablesShouldOnlyShareSamePagesForLastLevel) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0xbadddadcu;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt2(*gpu, &allocator, defaultMemoryBank);

    PageTable *ppgttForCloning[] = {&ppgtt2};

    stream.writeMemoryAndClonePageTables(&ppgtt1, ppgttForCloning, 1, gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536);

    EXPECT_NE(ppgtt1.getPhysicalAddress(), ppgtt2.getPhysicalAddress());

    auto pdpe1 = ppgtt1.getChild(ppgtt1.getIndex(gfxAddress));
    auto pdpe2 = ppgtt2.getChild(ppgtt2.getIndex(gfxAddress));
    EXPECT_NE(pdpe1->getPhysicalAddress(), pdpe2->getPhysicalAddress());

    auto pde1 = pdpe1->getChild(pdpe1->getIndex(gfxAddress));
    auto pde2 = pdpe2->getChild(pdpe2->getIndex(gfxAddress));
    EXPECT_NE(pde1->getPhysicalAddress(), pde2->getPhysicalAddress());

    auto pte1 = pde1->getChild(pde1->getIndex(gfxAddress));
    auto pte2 = pde2->getChild(pde2->getIndex(gfxAddress));
    EXPECT_NE(pte1->getPhysicalAddress(), pte2->getPhysicalAddress());

    auto page1 = pte1->getChild(pte1->getIndex(gfxAddress));
    auto page2 = pte2->getChild(pte2->getIndex(gfxAddress));
    EXPECT_EQ(page1->getPhysicalAddress(), page2->getPhysicalAddress());
    EXPECT_EQ(page1->isLocalMemory(), page2->isLocalMemory());
}

TEST_F(AubStreamTest, clonePageTablesShouldSetCorrectPageSize) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x5009000;

    PhysicalAddressAllocatorSimple allocator(2, 1, localMemorySupportedInTests);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    size_t pageSize = 4096;

    std::vector<PageInfo> entries{
        {0x2003000, sizeof(bytes), ppgtt.getMemoryBank() != MEMORY_BANK_SYSTEM, ppgtt.getMemoryBank()},
        {0x2004000, sizeof(bytes), ppgtt.getMemoryBank() != MEMORY_BANK_SYSTEM, ppgtt.getMemoryBank()}};

    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, sizeof(bytes), ppgtt.getMemoryBank(), 0, pageSize));
    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, pageSize + sizeof(bytes), ppgtt.getMemoryBank(), 0, pageSize));

    for (auto pageId = 0; pageId < 2; pageId++) {
        auto gpuAddress = gfxAddress + pageId * pageSize;
        auto pdpe = ppgtt.getChild(ppgtt.getIndex(gpuAddress));
        EXPECT_NE(nullptr, pdpe);

        auto pde = pdpe->getChild(pdpe->getIndex(gpuAddress));
        EXPECT_NE(nullptr, pde);

        auto pte = reinterpret_cast<PTE *>(pde->getChild(pde->getIndex(gpuAddress)));
        EXPECT_NE(nullptr, pte);
        EXPECT_EQ(pageSize, pte->getPageSize());

        auto page = pte->getChild(pte->getIndex(gpuAddress));
        EXPECT_EQ(entries[pageId].physicalAddress, page->getPhysicalAddress());
        EXPECT_EQ(entries[pageId].isLocalMemory, page->isLocalMemory());

        auto physicalAddress = PageTableHelper::getEntry(&ppgtt, gpuAddress);
        EXPECT_EQ(physicalAddress, page->getPhysicalAddress());
    }
}

TEST_F(AubStreamTest, writeMemoryReturnsPageTableEntriesWritten) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator(2, 1, localMemorySupportedInTests);
    PML4 ppgtt1(*gpu, &allocator, MEMORY_BANK_0);

    auto entriesWritten = stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 65536});
    EXPECT_EQ(1u, entriesWritten.size());

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);

    auto pageOffset = gfxAddress & (65536 - 1);
    EXPECT_EQ(physicalAddress, entriesWritten[0].physicalAddress - pageOffset);
    EXPECT_EQ(7u, entriesWritten[0].size);
    EXPECT_TRUE(entriesWritten[0].isLocalMemory);
}

TEST_F(AubStreamTest, freeMemoryShouldRemovePTEEntriesLocalMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, MEMORY_BANK_0);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);

    stream.freeMemory(&ppgtt1, gfxAddress, sizeof(bytes));

    auto pteEntry = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_EQ(0u, pteEntry);
}

TEST_F(AubStreamTest, freeMemoryShouldRemovePTEEntriesSystemMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);

    stream.freeMemory(&ppgtt1, gfxAddress, sizeof(bytes));

    auto pteEntry = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_EQ(0u, pteEntry);
}

TEST_F(AubStreamTest, givenPreviousPageTablesWhenNewPageSizeReservedWithTheSameGfxAddressThenEntryIsUpdated) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 64 * KB});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    auto pteEntry = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);
    EXPECT_TRUE(pteEntry & toBitValue(PpgttEntryBits::intermediatePageSizeBit));

    stream.freeMemory(&ppgtt1, gfxAddress, sizeof(bytes));

    // Change PageSize
    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 4 * KB});

    auto physicalAddress2 = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    auto pteEntry2 = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress2);
    EXPECT_NE(pteEntry, pteEntry2);
    EXPECT_FALSE(pteEntry2 & toBitValue(PpgttEntryBits::intermediatePageSizeBit));
}

TEST_F(AubStreamTest, givenMultiplePagesInSinglePteWhenPartOfMemoryFreedThenPteIsNotDeleted) {
    const size_t fullSize = 3 * 4 * KB;
    const size_t halfSize = fullSize / 2;
    auto bytes = std::make_unique<uint8_t[]>(fullSize);

    uint64_t gfxAddress = (1 << 20) - halfSize;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt1(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes.get(), fullSize, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 4 * KB});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    auto pteEntry = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);
    EXPECT_TRUE(pteEntry & toBitValue(PpgttEntryBits::presentBit));

    stream.freeMemory(&ppgtt1, gfxAddress, halfSize);

    physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    EXPECT_EQ(0u, physicalAddress);

    auto physicalAddress2 = PageTableHelper::getEntry(&ppgtt1, gfxAddress + halfSize);
    auto pteEntry2 = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress + halfSize);
    EXPECT_NE(0u, physicalAddress2);
    EXPECT_EQ(pteEntry, pteEntry2);
    EXPECT_TRUE(pteEntry2 & toBitValue(PpgttEntryBits::presentBit));
}

TEST_F(AubStreamTest, givenLocalMemoryWhenCloningMemoryThenPageWalkEntriesAreWrittenToStreamAndPTEIsNotWrittenWithMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);
    bool isLocalMemoryPage = true;

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4)).Times(1);

    EXPECT_CALL(stream, writeDiscontiguousPages(bytes, sizeof(bytes), _, DataTypeHintValues::TraceNotype)).Times(0);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, DataTypeHintValues::TraceNotype)).Times(0);

    std::vector<PageInfo> entries;
    PageInfo info = {0x2003000, sizeof(bytes), isLocalMemoryPage, ppgtt.getMemoryBank()};
    entries.push_back(info);

    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, 65536));
}

TEST_F(AubStreamTest, givenSystemMemoryWhenCloningMemoryThenPageWalkEntriesAreWrittenToStreamAndPTEIsNotWrittenWithMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_SYSTEM);
    bool isLocalMemoryPage = false;

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TracePml4Entry, DataTypeHintValues::TraceNotype)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TracePhysicalPdpEntry, DataTypeHintValues::TraceNotype)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TracePpgttPdEntry, DataTypeHintValues::TraceNotype)).Times(1);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype)).Times(1);

    EXPECT_CALL(stream, writeDiscontiguousPages(bytes, sizeof(bytes), _, DataTypeHintValues::TraceNotype)).Times(0);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, DataTypeHintValues::TraceNotype)).Times(0);

    std::vector<PageInfo> entries;
    PageInfo info = {0x2003000, sizeof(bytes), isLocalMemoryPage, ppgtt.getMemoryBank()};
    entries.push_back(info);

    stream.cloneMemory(&ppgtt, entries, AllocationParams(gfxAddress, nullptr, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, 65536));
}

TEST_F(AubStreamTest, givenLocalMemoryWhenWriteMemoryAndClonePageTablesIsCalledThenPagesAreWrittenToStream) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PML4 ppgtt(*gpu, &allocator, MEMORY_BANK_0);
    PML4 ppgttCloned(*gpu, &allocator, MEMORY_BANK_1);

    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(2);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(2);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3)).Times(2);
    EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4)).Times(2);

    EXPECT_CALL(stream, writeDiscontiguousPages(bytes, sizeof(bytes), _, DataTypeHintValues::TraceNotype)).Times(1);

    PageTable *ppgttForCloning[] = {&ppgttCloned};

    stream.writeMemoryAndClonePageTables(&ppgtt, ppgttForCloning, 1, gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 65536);
}

TEST_F(AubStreamTest, givenAllPossibilitiesToTbxTools) {
    EXPECT_EQ(IsAnyTbxMode(mode::aubFile), false);
    EXPECT_EQ(IsAnyTbxMode(mode::tbx), true);
    EXPECT_EQ(IsAnyTbxMode(mode::aubFileAndTbx), true);
    EXPECT_EQ(IsAnyTbxMode(mode::tbxShm), true);
    EXPECT_EQ(IsAnyTbxMode(mode::tbxShm3), true);
    EXPECT_EQ(IsAnyTbxShmMode(mode::aubFile), false);
    EXPECT_EQ(IsAnyTbxShmMode(mode::tbx), false);
    EXPECT_EQ(IsAnyTbxShmMode(mode::aubFileAndTbx), false);
    EXPECT_EQ(IsAnyTbxShmMode(mode::tbxShm), true);
    EXPECT_EQ(IsAnyTbxShmMode(mode::tbxShm3), true);
}

using AubStreamTest32 = AubStreamTest;

TEST_F(AubStreamTest32, freeMemoryShouldRemovePTEEntries) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PDP4 ppgtt1(*gpu, &allocator, defaultMemoryBank);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);

    stream.freeMemory(&ppgtt1, gfxAddress, sizeof(bytes));

    auto pteEntry = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_EQ(0u, pteEntry);
}

TEST_F(AubStreamTest32, givenPreviousPageTablesWhenNewPageSizeReservedWithTheSameGfxAddressThenEntryIsUpdated) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PDP4 ppgtt1(*gpu, &allocator, defaultMemoryBank);

    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 64 * KB});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    auto pteEntry = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);
    EXPECT_TRUE(pteEntry & toBitValue(PpgttEntryBits::intermediatePageSizeBit));

    stream.freeMemory(&ppgtt1, gfxAddress, sizeof(bytes));

    // Change PageSize
    stream.writeMemory(&ppgtt1, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 4 * KB});

    auto physicalAddress2 = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    auto pteEntry2 = PageTableHelper::getPTEEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress2);
    EXPECT_NE(pteEntry, pteEntry2);
    EXPECT_FALSE(pteEntry2 & toBitValue(PpgttEntryBits::intermediatePageSizeBit));
}

TEST_F(AubStreamTest32, writeMemoryWithNullPointerReservesMemoryInPPGTT) {
    uint64_t gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    PDP4 ppgtt1(*gpu, &allocator, defaultMemoryBank);

    // Shouldnt be writing any memory only reserving GPUVA
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, _)).Times(0);

    stream.writeMemory(&ppgtt1, {gfxAddress, nullptr, sizeof(uint32_t), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    auto physicalAddress = PageTableHelper::getEntry(&ppgtt1, gfxAddress);
    EXPECT_NE(0u, physicalAddress);
}

TEST_F(AubStreamTest32, writeMemoryWithNullPointerReservesMemoryInGGTT) {
    auto gfxAddress = 0x1000;

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);

    // Shouldnt be writing any memory only reserving GPUVA
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, _)).Times(0);

    stream.writeMemory(&ggtt, gfxAddress, nullptr, sizeof(uint32_t), defaultMemoryBank, DataTypeHintValues::TraceNotype, 4096);

    auto physicalAddress = PageTableHelper::getEntry(&ggtt, gfxAddress);
    EXPECT_NE(0u, physicalAddress);
}

TEST_F(AubStreamTest, givenGGTT64KBPageEntriesAre4KBAndArePhysicallyContiguous) {
    auto gfxAddress = 0x00000000;

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);

    // Force misalignment
    allocator.reservePhysicalMemory(defaultMemoryBank, 4096u, 65536u);

    stream.writeMemory(&ggtt, gfxAddress, nullptr, sizeof(uint32_t), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536u);

    auto physicalAddress = ggtt.getChild(0)->getPhysicalAddress();
    EXPECT_EQ(0u, physicalAddress & 0xffff);
    for (int i = 1; i < 16; i++) {
        auto child = ggtt.getChild(i);
        ASSERT_NE(nullptr, child);
        ASSERT_EQ(physicalAddress + i * 4096, child->getPhysicalAddress());
    }
}

using AubFileStreamTest = ::testing::Test;

HWTEST_F(AubFileStreamTest, givenCoreFamilyXeHpOrAboveWhenAubFileStreamIsInitializedThenDumpBinIsSupported, (HwMatcher::And<HwMatcher::coreAboveEqualXeHp, HwMatcher::coreBelowEqualXeHpgCore>)) {
    WhiteBox<AubFileStream> stream;
    stream.init(SteppingValues::A, *gpu);

    EXPECT_TRUE(stream.dumpBinSupported);
}

HWTEST_F(AubFileStreamTest, givenCoreFamilyXeHpOrAboveWhenAubFileStreamIsInitializedThenDumpSurfaceIsSupported, (HwMatcher::And<HwMatcher::coreAboveEqualXeHp, HwMatcher::coreBelowEqualXeHpgCore>)) {
    WhiteBox<AubFileStream> stream;
    stream.init(SteppingValues::A, *gpu);

    EXPECT_TRUE(stream.dumpSurfaceSupported);
}

TEST_F(AubFileStreamTest, givenErrorLogLevelWhenOperationFailsThenErrorIsPrinted) {
    if (Settings::disabled()) {
        GTEST_SKIP();
    }
    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->LogLevel.set(LogLevels::error);

    WhiteBox<AubFileStream> stream;

    errno = 0;
    stream.init(SteppingValues::A, *gpu);

    ::testing::internal::CaptureStdout();
    char temp = 0;
    stream.fileHandle.write(&temp, 1);
    stream.fileHandle.flush();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_THAT(output, ::testing::HasSubstr("[ERROR] write() to file failed\n"));
    EXPECT_THAT(output, ::testing::HasSubstr("[ERROR] flush() to file failed\n"));
}

TEST_F(AubFileStreamTest, givenThrowOnErrorWhenOperationFailsThenExcepionIsThrown) {
    WhiteBox<AubFileStream> stream;
    stream.enableThrowOnError(true);
    stream.init(SteppingValues::A, *gpu);
    char temp = 0;
    EXPECT_THROW(stream.fileHandle.write(&temp, 1);, std::runtime_error);
}
