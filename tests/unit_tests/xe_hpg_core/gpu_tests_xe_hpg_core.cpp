/*
 * Copyright (C) 2022-2024 Intel Corporation
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
    EXPECT_CALL(stream, writeMMIO(0x4900, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x4904, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x4908, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x490c, _)).Times(0);

    auto deviceCount = 1;
    auto memoryBankSize = 2 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}

TEST(Gpu, givenOneDeviceWhenSetGGTTBaseAddressThenIsProgrammedForOneTile) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpgCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x108100, 0xff500000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108104, 0x00000007)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108108, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x10810c, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108110, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108114, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108118, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x1108100, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x1108104, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x2108100, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x2108104, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x3108100, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x3108104, _)).Times(0);

    auto deviceCount = 1;
    auto memoryBankSize = 32ull * GB;
    auto dsmSize = 4ull * MB;
    auto sm = StolenMemory::CreateStolenMemory(false, deviceCount, memoryBankSize, dsmSize);
    gpu->setGGTTBaseAddresses(stream, deviceCount, memoryBankSize, *sm);
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

    EXPECT_CALL(stream, writeMMIO(_, _)).WillRepeatedly(Return());
    EXPECT_CALL(stream, writeMMIO(ccs.mmioEngine + 0x0209c, 0x10000000)).Times(1);
    ccs.initializeEngineMMIO(stream);
}
