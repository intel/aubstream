/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/settings.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include "test.h"
#include "gmock/gmock.h"
#include "test_defaults.h"
#include "tests/variable_backup.h"

using namespace aub_stream;
using ::testing::_;

TEST(GpuXe3p, givenXe3pWhenCheckingIndirectRingStateEnableThenTrueReturned) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe3pCore);

    EXPECT_TRUE(gpu->getCommandStreamerHelper(defaultDevice, defaultEngine).isRingDataEnabled());
}

TEST(GpuXe3p, givenXe3pWhenInitializingGlobalMmiosThenProgramCorrectRegisters) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe3pCore);
    MockAubFileStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AtLeast(0));

    EXPECT_CALL(stream, writeMMIO(0x00004b80, 0xffff1001)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00007000, 0xffff0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00007004, 0xffff0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00009008, 0x00000200)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000900c, 0x00001b40)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000b120, 0x14000002)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000b134, 0xa0000000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000b234, 0xa0000000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000ce90, 0x00030003)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000cf58, 0x80000000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000e194, 0xffff0002)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00014800, 0x00010001)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00014804, 0x0fff0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0001a0d8, 0x00020000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x00042080, 0x00000000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000e7c8, 0x00400000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x0000e7cc, 0x00001000)).Times(1);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0);
}
