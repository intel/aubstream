/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/misc_helpers.h"

using namespace aub_stream;

TEST(MiscHelpers, countBitsMustCalcCorrectValues) {
    EXPECT_EQ(1, countBits(1 << 1));
    EXPECT_EQ(1, countBits(1 << 5));
    EXPECT_EQ(1, countBits(1 << 14));
    EXPECT_EQ(2, countBits(1 << 3 | 1 << 13));
    EXPECT_EQ(2, countBits(1 << 2 | 1 << 11));
    EXPECT_EQ(2, countBits(1 << 2 | 1 << 4));
    EXPECT_EQ(2, countBits(1 << 2 | 1 << 3));
    EXPECT_EQ(2, countBits(1 << 12 | 1 << 13));
    EXPECT_EQ(3, countBits(1 | 1 << 5 | 1 << 12));
    EXPECT_EQ(3, countBits(1 << 2 | 1 << 4 | 1 << 15));
    EXPECT_EQ(3, countBits(1 << 3 | 1 << 9 | 1 << 10));
    EXPECT_EQ(5, countBits(1 << 1 | 1 << 5 | 1 << 13 | 1 << 14 | 1 << 15));
    EXPECT_EQ(5, countBits(1 << 1 | 1 << 5 | 1 << 13 | 1 << 14 | 1 << 15));
    EXPECT_EQ(5, countBits(1 << 0 | 1 << 3 | 1 << 8 | 1 << 11 | 1 << 13));
    EXPECT_EQ(8, countBits(0xff));
    EXPECT_EQ(8, countBits(0xff00));
    EXPECT_EQ(8, countBits(0x0ff0));
    EXPECT_EQ(8, countBits(0xf00f));
    EXPECT_EQ(8, countBits(0x0f0f));
    EXPECT_EQ(8, countBits(0xf0f0));
    EXPECT_EQ(8, countBits(0xcccc));
    EXPECT_EQ(8, countBits(0x3333));
    EXPECT_EQ(8, countBits(0xaaaa));
    EXPECT_EQ(12, countBits(0xff0f));
    EXPECT_EQ(16, countBits(0xffff));
}
