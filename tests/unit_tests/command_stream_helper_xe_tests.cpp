/*
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"

#include "aubstream/allocation_params.h"
#include "aubstream/engine_node.h"
#include "tests/unit_tests/command_stream_helper_tests.h"

#include "test.h"
#include "test_defaults.h"

#include <memory>
#include <vector>

using namespace aub_stream;

HWTEST_F(CommandStreamerHelperTest, WhenCommandStreamHelperIsInitializedThenLRCAIncludesContextSRWithInhibitContextRestoreAndSynchronousContextSwitch, HwMatcher::coreBelowEqualXeHpc) {
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);

    auto sizeLRCA = rcs.sizeLRCA;
    auto pLRCA = std::unique_ptr<uint32_t[]>(new uint32_t[rcs.sizeLRCA / sizeof(uint32_t)]);
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    rcs.initialize(reinterpret_cast<void *>(pLRCA.get()), &pageTable, 0);

    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2244, 0x00090009));
}

HWTEST_F(CommandStreamerHelperTest, WhenCommandStreamHelperIsInitializedWithFlagsThenLRCAIncludesContextSRWithSpecidfiedFlags, HwMatcher::coreBelowEqualXeHpc) {
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);

    auto sizeLRCA = rcs.sizeLRCA;
    uint32_t additionalFlags = 0x800080;
    auto pLRCA = std::unique_ptr<uint32_t[]>(new uint32_t[rcs.sizeLRCA / sizeof(uint32_t)]);
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    rcs.initialize(reinterpret_cast<void *>(pLRCA.get()), &pageTable, additionalFlags);

    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2244, (0x00090009 | additionalFlags)));
}
