/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/memory_bank_helper.h"

using namespace aub_stream;

TEST(MemoryBankHelper, whenZeroMemoryBankPassedThenGetMemoryBankReturnsZero) {
    MemoryBankHelper bankHelper(0, 0x1000, 0x1000);

    EXPECT_EQ(0u, bankHelper.getMemoryBank(0));
    EXPECT_EQ(0u, bankHelper.getMemoryBank(0x1000));
}

TEST(MemoryBankHelper, whenSinlgeMemoryBankPassedThenGetMemoryBankReturnsTheCorrectBank) {
    MemoryBankHelper bankHelper(2, 0x1000, 0x2000);

    EXPECT_EQ(2u, bankHelper.getMemoryBank(0x1000));
    EXPECT_EQ(2u, bankHelper.getMemoryBank(0x2000));
}

TEST(MemoryBankHelper, whenTwoMemoryBanksPassedThenNumberOfBanksIsCorrect) {
    MemoryBankHelper bankHelper(9, 0x1000, 0x2000);

    EXPECT_EQ(2u, bankHelper.numberOfBanks);
    EXPECT_EQ(1u, bankHelper.singleBanks[0]);
    EXPECT_EQ(8u, bankHelper.singleBanks[1]);
}

TEST(MemoryBankHelper, whenMultipleMemoryBanksPassedThenGetMemoryBankReturnsBankBasedOnCurrentAddressAndStepSize) {
    uint64_t initialGfxAddress = 0x1000;
    uint64_t gfxAddress = 0x1000;
    size_t numberOfBanks = 3;
    size_t totalSize = 0x3000;
    size_t stepSize = totalSize / numberOfBanks;

    MemoryBankHelper bankHelper(7, initialGfxAddress, totalSize);

    EXPECT_EQ(1u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += stepSize;
    EXPECT_EQ(2u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += stepSize;
    EXPECT_EQ(4u, bankHelper.getMemoryBank(gfxAddress));
}

TEST(MemoryBankHelper, givenMultipleMemoryBanksWhenPassingAddressWithinOneColorSizeThenGetMemoryBankReturnsCorrectBank) {
    uint64_t initialGfxAddress = 0x1000;
    uint64_t gfxAddress = 0x1000;
    size_t totalSize = 0x9000;
    MemoryBankHelper bankHelper(7, initialGfxAddress, totalSize);

    EXPECT_EQ(1u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += 0x1000;
    EXPECT_EQ(1u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += 0x1000;
    EXPECT_EQ(1u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += 0x1000;
    EXPECT_EQ(2u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += 0x1000;
    EXPECT_EQ(2u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += 0x1000;
    EXPECT_EQ(2u, bankHelper.getMemoryBank(gfxAddress));
    gfxAddress += 0x1000;
    EXPECT_EQ(4u, bankHelper.getMemoryBank(gfxAddress));
}

TEST(MemoryBankHelper, givenMultipleMemoryBanksWhenBankHelperIsCreatedThenCorrectColorSizeIsUsed) {
    uint64_t initialGfxAddress = 0x1000;
    size_t totalSize = 0x20000;
    MemoryBankHelper bankHelper(0xf, initialGfxAddress, totalSize);

    EXPECT_EQ(totalSize / 4, bankHelper.colorSize);
}
