/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST(Gpu, XeHpGivenOneDeviceSetMemoryBankSizeOnlyDefinesOneBank) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x4904, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x4908, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x490c, _)).Times(0);

    auto deviceCount = 1;
    auto memoryBankSize = 2 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}

TEST(Gpu, XeHpGivenTwoDevicesSetMemoryBankSizeOnlyDefinesTwoBanks) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, 0x00000201)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4904, 0x00000205)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4908, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x490c, _)).Times(0);

    auto deviceCount = 2;
    auto memoryBankSize = 2 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}

TEST(Gpu, XeHpGivenFourDeviceSetMemoryBankSizeDefinesAllBanks) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x4900, 0x00000201)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4904, 0x00000205)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4908, 0x00000209)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x490c, 0x0000020d)).Times(1);

    auto deviceCount = 4;
    auto memoryBankSize = 2 * GB;
    gpu->setMemoryBankSize(stream, deviceCount, memoryBankSize);
}

TEST(Gpu, XeHpFileStreamInitializeGlobalMMIOWritesFlatCcsBaseAddress) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);
    MockAubFileStream stream;
    auto sm = StolenMemory::CreateStolenMemory(false, 1, 1);
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x00004910, _)).Times(0);

    gpu->initializeDefaultMemoryPools(stream, 1, 1, *sm);
}

TEST(Gpu, XeHpTbxStreamInitializeGlobalMMIOWritesFlatCcsBaseAddress) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);
    MockTbxStream stream;
    auto sm = StolenMemory::CreateStolenMemory(false, 1, 1);
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x00004910, _)).Times(1);

    gpu->initializeDefaultMemoryPools(stream, 1, 1, *sm);
}

TEST(Gpu, XeHpInitializeGlobalMMIOWritesFlatCcsBaseAddressPtr) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);
    constexpr uint32_t mmioDeviceOffset = 16 * MB;
    constexpr uint32_t numDevices = 4;
    constexpr uint64_t perDeviceHbmSize = 8llu * GB;

    MockTbxStream stream;
    auto sm = StolenMemory::CreateStolenMemory(false, numDevices, perDeviceHbmSize);
    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));

    for (uint32_t i = 0; i > numDevices; i++) {
        uint64_t flatCcsBaseAddress = perDeviceHbmSize * (i + 1);            // device local memory ending
        flatCcsBaseAddress -= 9 * MB;                                        // wopcm and ggtt
        flatCcsBaseAddress -= (perDeviceHbmSize / 256);                      // flat_ccs size
        uint32_t mmioValue = static_cast<uint32_t>(flatCcsBaseAddress >> 8); // [8:31] base ptr
        mmioValue |= 1;                                                      // [0] enable bit

        EXPECT_CALL(stream, writeMMIO((i * mmioDeviceOffset) + 0x00004910, mmioValue)).Times(1);
    }

    gpu->initializeDefaultMemoryPools(stream, numDevices, perDeviceHbmSize, *sm);
}

TEST(Gpu, XeHpGivenOneDeviceThenGGTTBaseAddressIsProgrammedForOneTile) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x108100, 0xff800000)).Times(1);
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
    auto sm = StolenMemory::CreateStolenMemory(false, deviceCount, memoryBankSize);
    gpu->setGGTTBaseAddresses(stream, deviceCount, memoryBankSize, *sm);
}

TEST(Gpu, XeHpGivenTwoDevicesThenGGTTBaseAddressesAreProgrammedForTwoTiles) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x108100, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108104, 0x00000003)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108108, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x10810c, 0x00000007)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108110, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108114, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x108118, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x10811c, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x1108100, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x1108104, 0x00000007)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x2108100, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x2108104, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x3108100, _)).Times(0);
    EXPECT_CALL(stream, writeMMIO(0x3108104, _)).Times(0);

    auto deviceCount = 2;
    auto memoryBankSize = 16ull * GB;
    auto sm = StolenMemory::CreateStolenMemory(false, deviceCount, memoryBankSize);
    gpu->setGGTTBaseAddresses(stream, deviceCount, memoryBankSize, *sm);
}

TEST(Gpu, XeHpGivenFourDevicesThenGGTTBaseAddressesAreProgrammedForFourTiles) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpCore);

    MockAubStreamBase stream;
    EXPECT_CALL(stream, writeMMIO(0x108100, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108104, 0x00000001)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108108, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x10810c, 0x00000003)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108110, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108114, 0x00000005)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x108118, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x10811c, 0x00000007)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x1108100, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x1108104, 0x00000003)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x2108100, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x2108104, 0x00000005)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x3108100, 0xff800000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x3108104, 0x00000007)).Times(1);

    auto deviceCount = 4;
    auto memoryBankSize = 8ull * GB;
    auto sm = StolenMemory::CreateStolenMemory(false, deviceCount, memoryBankSize);
    gpu->setGGTTBaseAddresses(stream, deviceCount, memoryBankSize, *sm);
}
