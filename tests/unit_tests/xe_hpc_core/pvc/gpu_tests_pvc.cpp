/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hpc_core/pvc/command_streamer_helper_xe_hpc_core_pvc.h"
#include "test_defaults.h"
#include "unit_tests/mock_aub_stream.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST(Gpu, gpuPvcReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Pvc);

    EXPECT_EQ(0x27, gpu->deviceId);
}

TEST(Pvc, allocatePML5) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Pvc);
    PhysicalAddressAllocator allocator;
    GpuPvc gpu;

    auto ppgtt = std::unique_ptr<PageTable>(gpu.allocatePPGTT(&allocator, MEMORY_BANK_0, maxNBitValue(57)));
    EXPECT_EQ(57u, ppgtt->getNumAddressBits());
}

TEST(Pvc, givenInitializeGlobalMMIOWhenPvcSteppingA0ThenWritesMmioLtcdregWithSetSleepModeDisabled) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Pvc);
    MockAubStream stream;
    uint32_t LTCDREG = 0x0000b120;
    uint32_t valueWithSleepModeDisabled = 0x54000002;

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(LTCDREG, valueWithSleepModeDisabled)).Times(1);

    uint32_t stepping = SteppingValues::A;
    gpu->initializeGlobalMMIO(stream, 1, 1, stepping);
}

TEST(Pvc, givenInitializeGlobalMMIOWhenPvcSteppingNotA0ThenWritesMmioLtcdregWithoutSetSleepModeDisabled) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Pvc);
    MockAubStream stream;
    uint32_t LTCDREG = 0x0000b120;
    uint32_t valueWithSleepModeDisabled = 0x14000002;

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(LTCDREG, valueWithSleepModeDisabled)).Times(1);

    uint32_t stepping = SteppingValues::B;
    gpu->initializeGlobalMMIO(stream, 1, 1, stepping);
}