/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "test_defaults.h"
#include "gtest/gtest.h"

#include "test.h"

using namespace aub_stream;

TEST(PageTableTestsDg2, getEntryValue) {
    TEST_REQUIRES(gpu->productFamily == IGFX_DG2);

    uint64_t physicalAddress = 0x20000;
    PageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_0);

    auto extraEntryBits = gpu->getPPGTTExtraEntryBits({});
    EXPECT_EQ(toBitValue(PpgttEntryBits::atomicEnableBit), extraEntryBits);

    auto expectedEntryValue = physicalAddress |
                              toBitValue(PpgttEntryBits::atomicEnableBit, PpgttEntryBits::localMemoryBit,
                                         PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

    EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
}
