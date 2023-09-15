/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "tests/unit_tests/command_stream_helper_tests.h"

#include "test_defaults.h"
#include "gtest/gtest.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::Return;

struct XeHPCCoreMatcher {
    static bool isXeHpcCore(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily == aub_stream::CoreFamily::XeHpcCore;
    }
};

using XeHpcCoreCsTest = CommandStreamerHelperTest;
HWTEST_F(XeHpcCoreCsTest, GivenCcsEngineWhenInitializingEngineThenMiModeNestedBBIsSetToZero, XeHPCCoreMatcher::isXeHpcCore) {
    auto &ccs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_CCS);

    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);

    EXPECT_CALL(stream, writeMMIO(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(stream, writeMMIO(ccs.mmioEngine + 0x0209c, 0x10000000)).Times(1);
    ccs.initializeEngineMMIO(stream);
}
