/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"

#include "aubstream/headers/engine_node.h"
#include "tests/unit_tests/command_stream_helper_tests.h"

#include "test_defaults.h"
#include "test.h"

#include <memory>

using namespace aub_stream;

TEST_F(CommandStreamerHelperTest, givenDg2AndMemoryAlignment4KbWhenCheckIsMemorySupportedThenOnlyForMemoryBankSystemAndBank0IsReturnedTrue) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Dg2);
    auto &rcs = getCommandStreamerHelper(gpu->productFamily, defaultDevice, ENGINE_RCS);
    auto pageSize = 4096u;

    EXPECT_TRUE(rcs.isMemorySupported(MEMORY_BANK_SYSTEM, pageSize));
    EXPECT_TRUE(rcs.isMemorySupported(MEMORY_BANK_0, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_1, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_2, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_3, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_4, pageSize));
}

TEST_F(CommandStreamerHelperTest, givenDg2AndMemoryAlignment64KbWhenCheckIsMemorySupportedThenOnlyForMemoryBankSystemAndBank0IsReturnedTrue) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Dg2);
    auto &rcs = getCommandStreamerHelper(gpu->productFamily, defaultDevice, ENGINE_RCS);
    auto pageSize = 65536u;

    EXPECT_TRUE(rcs.isMemorySupported(MEMORY_BANK_SYSTEM, pageSize));
    EXPECT_TRUE(rcs.isMemorySupported(MEMORY_BANK_0, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_1, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_2, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_3, pageSize));
    EXPECT_FALSE(rcs.isMemorySupported(MEMORY_BANK_4, pageSize));
}
