/*
 * Copyright (C) 2024-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "tests/unit_tests/hardware_context_tests.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "test_defaults.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;

TEST_F(HardwareContextTest, givenContextWhenSubmittingThenSQSubmissionIsUsed) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::Xe3Core);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, 0);
    context0.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    for (int i = 0; i < 8; i++) {
        EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510 + (i * 8), _, 0xffffffff)).Times(1);
        EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514 + (i * 8), _, 0xffffffff)).Times(1);
    }
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2550, 1, 0xffffffff)).Times(1);

    context0.submitBatchBuffer(0x100, false);
}
