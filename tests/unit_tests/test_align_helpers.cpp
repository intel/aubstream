/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/align_helpers.h"

using namespace aub_stream;

TEST(AlignHelpers, alignUpMustDoCorrectCalculations) {
    EXPECT_EQ(alignUp(1, 5), 0x20);
    EXPECT_EQ(alignUp(1, 10), 0x400);
    EXPECT_EQ(alignUp(1, 20), 0x100000);
    EXPECT_EQ(alignUp(1, 30), 0x40000000);
    EXPECT_EQ(alignUp(0x20, 5), 0x20);
    EXPECT_EQ(alignUp(0x400, 10), 0x400);
    EXPECT_EQ(alignUp(0x100000, 20), 0x100000);
    EXPECT_EQ(alignUp(0x40000000, 30), 0x40000000);
    EXPECT_EQ(alignUp(0x21, 5), 0x40);
    EXPECT_EQ(alignUp(0x401, 10), 0x800);
    EXPECT_EQ(alignUp(0x100001, 20), 0x200000);
    EXPECT_EQ(alignUp(0x40000001, 30), 0x80000000);
}

TEST(AlignHelpers, alignDownMustDoCorrectCalculations) {
    EXPECT_EQ(alignDown(0x1, 30), 0);
    EXPECT_EQ(alignDown(0x10, 30), 0);
    EXPECT_EQ(alignDown(0x100, 30), 0);
    EXPECT_EQ(alignDown(0x1000, 30), 0);
    EXPECT_EQ(alignDown(0x80000000, 30), 0x80000000);
    EXPECT_EQ(alignDown(0x80000001, 30), 0x80000000);
    EXPECT_EQ(alignDown(0x80000500, 30), 0x80000000);
    EXPECT_EQ(alignDown(0x80400000, 30), 0x80000000);
    EXPECT_EQ(alignDown(0x86000000, 30), 0x80000000);
    EXPECT_EQ(alignDown(0xf6000000, 30), 0xc0000000);
    EXPECT_EQ(alignDown(0xf6000000, 31), 0x80000000);
    EXPECT_EQ(alignDown(0x80000000, 31), 0x80000000);
}
