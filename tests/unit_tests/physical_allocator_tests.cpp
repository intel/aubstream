/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/physical_address_allocator.h"
#include "mock_physical_address_allocator.h"

using namespace aub_stream;

TEST(PhysicalAddressAllocator, whenDefaultCtorIsUsedThenNumberOfCreatedAllocatorsIsZero) {
    MockPhysicalAddressAllocatorSimple allocator;

    EXPECT_EQ(0u, allocator.numberOfAllocators);
    EXPECT_EQ(0u, allocator.allocators.size());
}

TEST(PhysicalAddressAllocator, whenTwoAllocatorsArePassedToCtorThenTwoAllocatorsAreCreated) {
    MockPhysicalAddressAllocatorSimple allocator1(2, 4, false);
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper allocator2(4, 8, false, nullptr);

    EXPECT_EQ(2u, allocator1.numberOfAllocators);
    EXPECT_EQ(2u, allocator1.allocators.size());
    EXPECT_EQ(4u, allocator2.numberOfAllocators);
    EXPECT_EQ(4u, allocator2.allocators.size());
}

TEST(PhysicalAddressAllocator, givenAllocatorWithTwoAllocatorsWhenPhysicalMemoryForMemoryBanksIsReservedThenAddressesFromBanksAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocatorSimple allocator(2, allocatorSize, false);

    auto physicalMemory1 = allocator.reservePhysicalMemory(1, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(2, 4096, 4096);

    EXPECT_EQ(0x1000u, physicalMemory1);
    EXPECT_EQ(allocatorSize, physicalMemory2);
}

TEST(PhysicalAddressAllocator, givenAllocatorWithTwoAllocatorsWhenPhysicalMemoryForMemoryBanksIsFreedThenCorrectAllocatorIsUsed) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocatorSimple allocator(2, allocatorSize, false);

    auto simpleAllocator0 = new MockSimpleAllocator<uint64_t>(0x4000);
    allocator.allocators[0].reset(simpleAllocator0);

    auto simpleAllocator1 = new MockSimpleAllocator<uint64_t>(0x16000);
    allocator.allocators[1].reset(simpleAllocator1);

    auto physicalMemory = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_SYSTEM, 4096, 4096);
    auto physicalMemory1 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 4096, 4096);

    allocator.freePhysicalMemory(MemoryBank::MEMORY_BANK_SYSTEM, physicalMemory);

    allocator.freePhysicalMemory(MemoryBank::MEMORY_BANK_0, physicalMemory1);
    EXPECT_TRUE(simpleAllocator0->alignedFreeCalled);

    allocator.freePhysicalMemory(MemoryBank::MEMORY_BANK_1, physicalMemory2);
    EXPECT_TRUE(simpleAllocator1->alignedFreeCalled);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorWithSpecifiedMemoryBankSizeWhenMemoryFromBanksIsReservedThenCorrectAddressesAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocatorSimple allocator(3, allocatorSize, false);

    auto physicalMemory1 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 4096, 4096);
    auto physicalMemory3 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_2, 4096, 4096);

    EXPECT_EQ(0x1000u, physicalMemory1);
    EXPECT_EQ(allocatorSize, physicalMemory2);
    EXPECT_EQ(2 * allocatorSize, physicalMemory3);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorWithLocalMemorySupportAndSpecifiedMemoryBankSizeWhenMemoryFromBanksIsReservedThenCorrectAddressesAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocatorSimple allocator(3, allocatorSize, true);

    auto physicalMemory1 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 4096, 4096);
    auto physicalMemory3 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_2, 4096, 4096);

    const auto reservedPhysicalMemory = allocatorSize / 256;

    EXPECT_EQ(reservedPhysicalMemory, physicalMemory1);
    EXPECT_EQ(allocatorSize + reservedPhysicalMemory, physicalMemory2);
    EXPECT_EQ(2 * allocatorSize + reservedPhysicalMemory, physicalMemory3);
}

TEST(PhysicalAddressAllocator, givenHeapBasedAllocatorWhenReservingMemoryThenAllocatedMemoryIsStoredForRelease) {
    MockPhysicalAddressAllocatorHeap allocator;

    auto p1 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    EXPECT_EQ(allocator.storage.size(), 1);
    auto i = allocator.storage.begin();
    EXPECT_EQ(p1, reinterpret_cast<uint64_t>(i->get()));
    auto p2 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 1024, 1024);
    EXPECT_EQ(allocator.storage.size(), 2);
    i++;
    EXPECT_EQ(p2, reinterpret_cast<uint64_t>(i->get()));
    auto p3 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_2, 512, 512);
    EXPECT_EQ(allocator.storage.size(), 3);
    i++;
    EXPECT_EQ(p3, reinterpret_cast<uint64_t>(i->get()));
}

TEST(PhysicalAddressAllocator, whenPhysicalAllocatorIsCreatedWithInHeapParameterThenPhysicalAddressAllocatorHeapIsCreated) {
    auto allocatorHeap = std::make_unique<PhysicalAddressAllocatorHeap>();

    auto ph1 = allocatorHeap->reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 5, 4096);
    auto ph2 = allocatorHeap->reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 5, 4096);

    EXPECT_NE(ph1, ph2);
}

TEST(PhysicalAddressAllocator, whenPhysicalAllocatorIsCreatedWithoutInHeapParameterThenPhysicalAddressAllocatorSimpleIsCreated) {
    auto allocatorSimple = std::make_unique<PhysicalAddressAllocatorSimple>(
        2, 0x1000, false);

    auto ps1 = allocatorSimple->reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 5, 4096);
    auto ps2 = allocatorSimple->reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 5, 4096);

    EXPECT_EQ(ps1, ps2);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorForSHM4WithSpecifiedMemoryBankSizeWhenMemoryFromBanksIsReservedThenCorrectAddressesAreReturnedAndProperMappingsAreCreated) {
    const uint64_t allocatorSize = 0x100000;
    uint8_t *translationTab[allocatorSize * 3 / 0x1000] = {nullptr};
    uint8_t *translationLTab[allocatorSize * 3 / 0x1000] = {nullptr};
    SharedMemoryInfo sharedMemoryInfo = {
        reinterpret_cast<uint8_t *>(translationTab), allocatorSize, reinterpret_cast<uint8_t *>(translationLTab), allocatorSize};
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper allocator0(3, allocatorSize, true, &sharedMemoryInfo);
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper allocator1(0, allocatorSize, true, &sharedMemoryInfo);

    auto physicalMemory0 = allocator0.reservePhysicalMemoryBase(MemoryBank::MEMORY_BANK_SYSTEM, 4096, 4096);
    auto physicalMemory1 = allocator0.reservePhysicalMemoryBase(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory2 = allocator0.reservePhysicalMemoryBase(MemoryBank::MEMORY_BANK_1, 2 * 4096, 4096);
    auto physicalMemory3 = allocator0.reservePhysicalMemoryBase(MemoryBank::MEMORY_BANK_2, 4096, 4096);
    auto physicalMemory4 = allocator1.reservePhysicalMemoryBase(MemoryBank::MEMORY_BANK_2, 4096, 4096);

    void *pointer0 = nullptr;
    void *pointer1 = nullptr;
    void *pointer2 = nullptr;
    void *pointer3 = nullptr;
    void *pointer4 = nullptr;
    size_t availableSize0 = 0;
    size_t availableSize1 = 0;
    size_t availableSize2 = 0;
    size_t availableSize3 = 0;
    size_t availableSize4 = 0;

    allocator0.translatePhysicalAddressToSystemMemoryBase(physicalMemory0, 1 * 4096, false, pointer0, availableSize0);
    allocator0.translatePhysicalAddressToSystemMemoryBase(physicalMemory1, 2 * 4096, true, pointer1, availableSize1);
    allocator0.translatePhysicalAddressToSystemMemoryBase(physicalMemory2, 3 * 4096, true, pointer2, availableSize2);
    allocator0.translatePhysicalAddressToSystemMemoryBase(physicalMemory3, 4 * 4096, true, pointer3, availableSize3);
    allocator0.translatePhysicalAddressToSystemMemoryBase(physicalMemory3 + 0x1001, 4096, true, pointer4, availableSize4);

    EXPECT_EQ(reservedGGTTSpace, physicalMemory0);
    EXPECT_EQ(0x1000u, physicalMemory1);
    EXPECT_EQ(allocatorSize + 0x1000, physicalMemory2);
    EXPECT_EQ(2 * allocatorSize + 0x1000, physicalMemory3);
    EXPECT_EQ(reservedGGTTSpace, physicalMemory4);

    EXPECT_NE(translationTab[physicalMemory0 / 0x1000], nullptr);
    EXPECT_EQ(translationTab[physicalMemory0 / 0x1000 + 1], nullptr);
    EXPECT_NE(translationLTab[physicalMemory1 / 0x1000], nullptr);
    EXPECT_EQ(translationLTab[physicalMemory1 / 0x1000 + 1], nullptr);
    EXPECT_NE(translationLTab[physicalMemory2 / 0x1000], nullptr);
    EXPECT_NE(translationLTab[physicalMemory2 / 0x1000 + 1], nullptr);
    EXPECT_EQ(translationLTab[physicalMemory2 / 0x1000 + 2], nullptr);
    EXPECT_NE(translationLTab[physicalMemory3 / 0x1000], nullptr);
    EXPECT_EQ(translationLTab[physicalMemory3 / 0x1000 + 1], nullptr);

    EXPECT_EQ(translationTab[physicalMemory0 / 0x1000], pointer0);
    EXPECT_EQ(translationLTab[physicalMemory1 / 0x1000], pointer1);
    EXPECT_EQ(translationLTab[physicalMemory2 / 0x1000], pointer2);
    EXPECT_EQ(translationLTab[physicalMemory3 / 0x1000], pointer3);
    EXPECT_EQ(nullptr, pointer4);
    EXPECT_EQ(availableSize0, 4096);
    EXPECT_EQ(availableSize1, 4096);
    EXPECT_EQ(availableSize2, 2 * 4096);
    EXPECT_EQ(availableSize3, 4096);
    EXPECT_EQ(availableSize4, 0);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorForSHM4WithSpecifiedMemoryBankSizeWhenMemoryFromBanksIsSpaceOnlyReservedThenCorrectAddressesAreReturnedAndNoMappingsAreCreated) {
    const uint64_t allocatorSize = 0x100000;
    uint8_t *translationTab[allocatorSize / 0x1000] = {nullptr};
    uint8_t *translationLTab[allocatorSize / 0x1000] = {nullptr};
    SharedMemoryInfo sharedMemoryInfo = {
        reinterpret_cast<uint8_t *>(translationTab), allocatorSize, reinterpret_cast<uint8_t *>(translationLTab), allocatorSize};
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper allocator0(1, allocatorSize, true, &sharedMemoryInfo);
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper allocator1(0, allocatorSize, true, &sharedMemoryInfo);

    auto physicalMemory00 = allocator0.reserveOnlyPhysicalSpace(MemoryBank::MEMORY_BANK_SYSTEM, 4096, 4096);
    auto physicalMemory01 = allocator0.reserveOnlyPhysicalSpace(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory10 = allocator1.reserveOnlyPhysicalSpace(MemoryBank::MEMORY_BANK_SYSTEM, 4096, 4096);
    auto physicalMemory11 = allocator1.reserveOnlyPhysicalSpace(MemoryBank::MEMORY_BANK_0, 4096, 4096);

    EXPECT_EQ(reservedGGTTSpace, physicalMemory00);
    EXPECT_EQ(0x1000u, physicalMemory01);
    EXPECT_EQ(reservedGGTTSpace, physicalMemory10);
    EXPECT_EQ(reservedGGTTSpace + 0x1000, physicalMemory11);

    EXPECT_EQ(translationLTab[physicalMemory01 / 0x1000], nullptr);
    EXPECT_EQ(translationLTab[physicalMemory01 / 0x1000 + 1], nullptr);
    EXPECT_EQ(translationTab[physicalMemory00 / 0x1000], nullptr);
    EXPECT_EQ(translationTab[physicalMemory00 / 0x1000 + 1], nullptr);

    EXPECT_EQ(translationLTab[physicalMemory11 / 0x1000], nullptr);
    EXPECT_EQ(translationLTab[physicalMemory11 / 0x1000 + 1], nullptr);
    EXPECT_EQ(translationTab[physicalMemory10 / 0x1000], nullptr);
    EXPECT_EQ(translationTab[physicalMemory10 / 0x1000 + 1], nullptr);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorForSHM4WithSpecifiedMemoryBankSizeWhenMemoryFromBanksIsSpaceOnlyReservedAndMapOwnPageThenCorrectAddressesAreReturnedAndappingsAreCreated) {
    const uint64_t allocatorSize = 0x100000;
    uint8_t *translationTab[allocatorSize / 0x1000] = {nullptr};
    uint8_t *translationLTab[allocatorSize / 0x1000] = {nullptr};
    SharedMemoryInfo sharedMemoryInfo = {
        reinterpret_cast<uint8_t *>(translationTab), allocatorSize, reinterpret_cast<uint8_t *>(translationLTab), allocatorSize};
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper allocator(1, allocatorSize, true, &sharedMemoryInfo);

    auto physicalMemory0 = allocator.reserveOnlyPhysicalSpace(MemoryBank::MEMORY_BANK_SYSTEM, 4096, 4096);
    auto physicalMemory1 = allocator.reserveOnlyPhysicalSpace(MemoryBank::MEMORY_BANK_0, 4096, 4096);

    uint8_t p0[4096];
    uint8_t p1[4096];

    allocator.mapSystemMemoryToPhysicalAddress(physicalMemory0, 4096, 4096, false, p0);
    allocator.mapSystemMemoryToPhysicalAddress(physicalMemory1, 4096, 4096, true, p1);

    EXPECT_EQ(reservedGGTTSpace, physicalMemory0);
    EXPECT_EQ(0x1000u, physicalMemory1);

    EXPECT_EQ(translationLTab[physicalMemory1 / 0x1000], p1);
    EXPECT_EQ(translationLTab[physicalMemory1 / 0x1000 + 1], nullptr);
    EXPECT_EQ(translationTab[physicalMemory0 / 0x1000], p0);
    EXPECT_EQ(translationTab[physicalMemory0 / 0x1000 + 1], nullptr);
}