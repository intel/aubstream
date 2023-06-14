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
    MockPhysicalAddressAllocatorSimple allocator(2, 4, false);

    EXPECT_EQ(2u, allocator.numberOfAllocators);
    EXPECT_EQ(2u, allocator.allocators.size());
}

TEST(PhysicalAddressAllocator, givenAllocatorWithTwoAllocatorsWhenPhysicalMemoryForMemoryBanksIsReservedThenAddressesFromBanksAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocatorSimple allocator(2, allocatorSize, false);

    auto physicalMemory1 = allocator.reservePhysicalMemory(1, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(2, 4096, 4096);

    EXPECT_EQ(0x1000u, physicalMemory1);
    EXPECT_EQ(allocatorSize, physicalMemory2);
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
    auto allocatorHeap = PhysicalAddressAllocator::CreatePhysicalAddressAllocator(true, 2, 0x1000, false);

    auto ph1 = allocatorHeap->reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 5, 4096);
    auto ph2 = allocatorHeap->reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 5, 4096);

    EXPECT_NE(ph1, ph2);
}

TEST(PhysicalAddressAllocator, whenPhysicalAllocatorIsCreatedWithoutInHeapParameterThenPhysicalAddressAllocatorSimpleIsCreated) {
    auto allocatorSimple = PhysicalAddressAllocator::CreatePhysicalAddressAllocator(false, 2, 0x1000, false);

    auto ps1 = allocatorSimple->reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 5, 4096);
    auto ps2 = allocatorSimple->reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 5, 4096);

    EXPECT_EQ(ps1, ps2);
}