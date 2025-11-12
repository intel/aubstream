/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"

#include "aubstream/engine_node.h"
#include "tests/unit_tests/command_stream_helper_tests.h"

#include "test.h"
#include "test_defaults.h"

#include <vector>

using namespace aub_stream;

bool coreEqualXe3p(const aub_stream::Gpu *gpu) {
    return gpu->gfxCoreFamily == aub_stream::CoreFamily::Xe3pCore;
}

HWTEST_F(CommandStreamerHelperTest, CheckCCSFlushCommandsXe3p, coreEqualXe3p) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS));

    auto &ccs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_CCS);
    std::vector<uint32_t> testRingBuffer{};

    ccs.addFlushCommands(testRingBuffer);

    size_t sizeOfCommands = 0;
    // Verify the first Pipe Control
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x7a001004); // QueueDrainMode set
    // We are testing for:
    // no CSStallEnable
    // DCFlushEnable
    // IndirectStatePointersDisable
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x00000220);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Verify the second Pipe Control
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x7a001004);
    // We are testing for:
    // no CSStallEnable
    // IndirectStatePointersDisable
    // StateCacheInvalidationEnable
    // ConstantCacheInvalidationEnable
    // DCFlushEnable
    // TextureCacheInvalidateEnable
    // InstructionCacheInvalidateEnable
    // CommandCacheInvalidateEnable
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x20000E2C);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}
