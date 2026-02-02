/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "tests/unit_tests/command_stream_helper_tests.h"

#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

TEST(Gpu, givenOneDeviceWhenSetMemoryBankSizeThenOnlyDefinedOneBank) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpgCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x4904, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x4908, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x490c, _, 0xffffffff)).Times(0);

    auto deviceCount = 1;
    auto memoryBankSize = 2 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}

TEST(Gpu, givenOneDeviceWhenSetGGTTBaseAddressThenIsProgrammedForOneTile) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpgCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x108100, 0xff000000, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108104, 0x00000007, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108108, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x10810c, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108110, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108114, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108118, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x1108100, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x1108104, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x2108100, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x2108104, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x3108100, _, 0xffffffff)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x3108104, _, 0xffffffff)).Times(0);

    auto deviceCount = 1;
    auto memoryBankSize = 32ull * GB;
    gpu->stolenMemory = StolenMemory::CreateStolenMemory(false, deviceCount, memoryBankSize, gpu->getStolenMemorySize(memoryBankSize));
    gpu->setGGTTBaseAddresses(stream, deviceCount, memoryBankSize);
}

struct XeHPGCoreMatcher {
    static bool isXeHpgCore(const aub_stream::Gpu *gpu) {
        return gpu->gfxCoreFamily == aub_stream::CoreFamily::XeHpgCore;
    }
};

using XeHpgCoreCsTest = CommandStreamerHelperTest;
HWTEST_F(XeHpgCoreCsTest, GivenCcsEngineWhenInitializingEngineThenMiModeNestedBBIsSetToZero, XeHPGCoreMatcher::isXeHpgCore) {
    auto &ccs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_CCS);

    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);

    EXPECT_CALL(stream, writeMMIO(_, _, 0xffffffff)).WillRepeatedly(Return());
    EXPECT_CALL(stream, writeMMIO(ccs.mmioEngine + 0x0209c, 0x10000000, 0xffffffff)).Times(1);
    ccs.initializeEngineMMIO(stream);
}
