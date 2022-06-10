/*
 * Copyright (C) 2022 Intel Corporation
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
    MockPhysicalAddressAllocator allocator;

    EXPECT_EQ(0u, allocator.numberOfAllocators);
    EXPECT_EQ(0u, allocator.allocators.size());
}

TEST(PhysicalAddressAllocator, whenTwoAllocatorsArePassedToCtorThenTwoAllocatorsAreCreated) {
    MockPhysicalAddressAllocator allocator(2, 4, false);

    EXPECT_EQ(2u, allocator.numberOfAllocators);
    EXPECT_EQ(2u, allocator.allocators.size());
}

TEST(PhysicalAddressAllocator, givenAllocatorWithTwoAllocatorsWhenPhysicalMemoryForMemoryBanksIsReservedThenAddressesFromBanksAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocator allocator(2, allocatorSize, false);

    auto physicalMemory1 = allocator.reservePhysicalMemory(1, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(2, 4096, 4096);

    EXPECT_EQ(0x1000u, physicalMemory1);
    EXPECT_EQ(allocatorSize, physicalMemory2);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorWithSpecifiedMemoryBankSizeWhenMemoryFromBanksIsReservedThenCorrectAddressesAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocator allocator(3, allocatorSize, false);

    auto physicalMemory1 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 4096, 4096);
    auto physicalMemory3 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_2, 4096, 4096);

    EXPECT_EQ(0x1000u, physicalMemory1);
    EXPECT_EQ(allocatorSize, physicalMemory2);
    EXPECT_EQ(2 * allocatorSize, physicalMemory3);
}

TEST(PhysicalAddressAllocator, givenPhysicalAllocatorWithLocalMemorySupportAndSpecifiedMemoryBankSizeWhenMemoryFromBanksIsReservedThenCorrectAddressesAreReturned) {
    const auto allocatorSize = 1 * GB;

    MockPhysicalAddressAllocator allocator(3, allocatorSize, true);

    auto physicalMemory1 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_0, 4096, 4096);
    auto physicalMemory2 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_1, 4096, 4096);
    auto physicalMemory3 = allocator.reservePhysicalMemory(MemoryBank::MEMORY_BANK_2, 4096, 4096);

    const auto reservedPhysicalMemory = allocatorSize / 256;

    EXPECT_EQ(reservedPhysicalMemory, physicalMemory1);
    EXPECT_EQ(allocatorSize + reservedPhysicalMemory, physicalMemory2);
    EXPECT_EQ(2 * allocatorSize + reservedPhysicalMemory, physicalMemory3);
}
