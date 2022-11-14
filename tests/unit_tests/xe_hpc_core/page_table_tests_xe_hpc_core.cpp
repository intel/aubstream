/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/xe_hpc_core/command_streamer_helper_xe_hpc_core.h"

#include "test_defaults.h"
#include "gtest/gtest.h"

#include "test.h"

using namespace aub_stream;

struct MockPageTableMemory : PageTableMemory {
    using PageTableMemory::additionalAllocParams;
    using PageTableMemory::PageTableMemory;
};

TEST(PageTableTestsXeHpcCore, getEntryValue) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpcCore);

    uint64_t physicalAddress = 0x20000;
    MockPageTableMemory pageTable(*gpu, physicalAddress, MEMORY_BANK_0);

    // cached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = false;

        pageTable.additionalAllocParams = params;

        auto expectedExtraBits = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXeHpcCore::patIndex3;

        auto extraEntryBits = gpu->getPPGTTExtraEntryBits(params);
        EXPECT_EQ(expectedExtraBits, extraEntryBits);

        auto expectedEntryValue = physicalAddress | expectedExtraBits |
                                  toBitValue(PpgttEntryBits::localMemoryBit,
                                             PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
    }

    // uncached
    {
        AllocationParams::AdditionalParams params = {};
        params.uncached = true;

        pageTable.additionalAllocParams = params;

        auto expectedExtraBits = toBitValue(PpgttEntryBits::atomicEnableBit) | GpuXeHpcCore::patIndex0;

        auto extraEntryBits = gpu->getPPGTTExtraEntryBits(params);
        EXPECT_EQ(expectedExtraBits, extraEntryBits);

        auto expectedEntryValue = physicalAddress | expectedExtraBits |
                                  toBitValue(PpgttEntryBits::localMemoryBit,
                                             PpgttEntryBits::writableBit, PpgttEntryBits::presentBit);

        EXPECT_EQ(expectedEntryValue, pageTable.getEntryValue());
    }
}
