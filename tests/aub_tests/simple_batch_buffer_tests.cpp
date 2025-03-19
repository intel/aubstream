/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_file_stream.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/tbx_stream.h"
#include "aubstream/aubstream.h"
#include "simple_batch_buffer_fixture.h"
#include "test_defaults.h"
#include "tests/simple_batch_buffer.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "gtest/gtest.h"
#include <algorithm>

#include "test.h"

using namespace aub_stream;

TEST_P(SimpleBatchBuffer, rcs) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_RCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, bcs) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_BCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, vcs) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_VCS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_VCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, vecs) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_VECS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_VECS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, ccs) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_CCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, dualBatchBufferRcs) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_RCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

HWTEST_P(SimpleBatchBuffer, tile1Rcs, MatchMultiDevice::moreThanOne) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    desc.deviceCount = 4;
    initializeStream(desc);

    uint32_t device = 1;
    ctxt = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

HWTEST_P(SimpleBatchBuffer, tile2Rcs, MatchMultiDevice::moreThanTwo) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    desc.deviceCount = 4;
    initializeStream(desc);

    uint32_t device = 2;
    ctxt = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

HWTEST_P(SimpleBatchBuffer, tile3Rcs, MatchMultiDevice::moreThanThree) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    desc.deviceCount = 4;
    initializeStream(desc);

    uint32_t device = 3;
    ctxt = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, dualContext) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_RCS, 0);
    auto ccs = mgr->createHardwareContext(defaultDevice, ENGINE_CCS, 0);

    addSimpleBatchBuffer(ctxt, GetParam());
    addSimpleBatchBuffer(ccs, GetParam());

    ctxt->pollForCompletion();
    ccs->pollForCompletion();

    delete ccs;
}

static uint32_t batchBufferMemoryBanks[] = {
    MEMORY_BANK_SYSTEM,
    MEMORY_BANK_0,
    MEMORY_BANK_3,
};

std::string memoryBanksParamToString(::testing::TestParamInfo<uint32_t> paramInfo) {
    return memoryBanksToString(paramInfo.param);
}

INSTANTIATE_TEST_SUITE_P(,
                         SimpleBatchBuffer,
                         ::testing::ValuesIn(batchBufferMemoryBanks),
                         memoryBanksParamToString);
