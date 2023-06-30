/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "mock_stolen_memory.h"

using namespace aub_stream;

TEST(StolenMemory, whenStaticStolenMemoryCreatedThenFixedBaseAddressesAssigned) {
    MockStolenMemoryInStaticStorage stolenMemoryStatic1(0x80000000);
    MockStolenMemoryInStaticStorage stolenMemoryStatic2(0x80000000);

    EXPECT_EQ(stolenMemoryStatic1.staticMemoryBankSize, 0x80000000);
    EXPECT_EQ(stolenMemoryStatic2.staticMemoryBankSize, 0x80000000);

    EXPECT_EQ(stolenMemoryStatic1.getBaseAddress(0), stolenMemoryStatic2.getBaseAddress(0));
}

TEST(StolenMemory, whenStaticStolenMemoryCreatedBaseAddressMustBeAlways1MBAligned) {
    MockStolenMemoryInStaticStorage stolenMemoryStatic1(0x1000000100);
    MockStolenMemoryInStaticStorage stolenMemoryStatic2(0x1000002000);
    MockStolenMemoryInStaticStorage stolenMemoryStatic3(0x1000030000);
    MockStolenMemoryInStaticStorage stolenMemoryStatic4(0x1000400000);

    const uint64_t mask = (uint64_t(1) << 20) - 1;

    EXPECT_EQ(stolenMemoryStatic1.getBaseAddress(0) & mask, 0);
    EXPECT_EQ(stolenMemoryStatic2.getBaseAddress(0) & mask, 0);
    EXPECT_EQ(stolenMemoryStatic3.getBaseAddress(0) & mask, 0);
    EXPECT_EQ(stolenMemoryStatic4.getBaseAddress(1) & mask, 0);
}

TEST(StolenMemory, whenHeapBasedStolenMemoryCreatedBaseAddressMustBeAlways1MBAligned) {
    MockStolenMemoryInHeap stolenMemoryHeap1(1, 0x10000100);
    MockStolenMemoryInHeap stolenMemoryHeap2(1, 0x10002000);
    MockStolenMemoryInHeap stolenMemoryHeap3(1, 0x10030000);
    MockStolenMemoryInHeap stolenMemoryHeap4(1, 0x10400000);

    const uint64_t mask = (uint64_t(1) << 20) - 1;

    EXPECT_EQ(stolenMemoryHeap1.getBaseAddress(0) & mask, 0);
    EXPECT_EQ(stolenMemoryHeap2.getBaseAddress(0) & mask, 0);
    EXPECT_EQ(stolenMemoryHeap3.getBaseAddress(0) & mask, 0);
    EXPECT_EQ(stolenMemoryHeap4.getBaseAddress(0) & mask, 0);
}

TEST(StolenMemory, whenHeapBasedStolenMemoryCreatedThenBaseAddressesAllocatedWithSystemHeapAllocator) {
    MockStolenMemoryInHeap stolenMemoryHeap1(1, 0x80000000);
    MockStolenMemoryInHeap stolenMemoryHeap2(1, 0x80000000);

    EXPECT_EQ(stolenMemoryHeap1.getBaseAddress(0), reinterpret_cast<uint64_t>(stolenMemoryHeap1.localStolenStorage[0].get()));
    EXPECT_EQ(stolenMemoryHeap2.getBaseAddress(0), reinterpret_cast<uint64_t>(stolenMemoryHeap2.localStolenStorage[0].get()));
    EXPECT_NE(stolenMemoryHeap1.getBaseAddress(0), stolenMemoryHeap2.getBaseAddress(0));
}

TEST(StolenMemory, whenTwoStaticStolenMemoryCreatedThenFixedBaseAddressesAssigned) {
    auto stolenMemoryStatic = StolenMemory::CreateStolenMemory(false, 2, 0x80000000);

    EXPECT_LT(stolenMemoryStatic->getBaseAddress(0), stolenMemoryStatic->getBaseAddress(1));
    EXPECT_EQ(stolenMemoryStatic->getBaseAddress(1) - stolenMemoryStatic->getBaseAddress(0), 0x80000000);
}

TEST(StolenMemory, whenTwoHeapBasedStolenMemoryCreatedThenBaseAddressesAllocatedWithSystemHeapAllocatorReturnsAlwaysDifferentAddresses) {
    auto stolenMemoryHeap = StolenMemory::CreateStolenMemory(true, 2, 0x80000000);

    EXPECT_NE(stolenMemoryHeap->getBaseAddress(0), stolenMemoryHeap->getBaseAddress(1));
}