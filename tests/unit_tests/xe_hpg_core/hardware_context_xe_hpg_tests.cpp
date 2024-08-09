/*
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/settings.h"
#include "tests/variable_backup.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "tests/unit_tests/hardware_context_tests.h"
#include "test_defaults.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;

TEST_F(HardwareContextTest, givenExeclistSubmitPortSubmissionEnabledWhenSubmittingThenUseExeclistPortSubmission) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpgCore);

    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->ExeclistSubmitPortSubmission.set(1);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, nullptr, 0);
    context0.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2230, _)).Times(16);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2550, 1)).Times(1);

    context0.submitBatchBuffer(0x100, false);
}
