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
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_BCS));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_BCS, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, bcs1) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_BCS1));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_BCS1, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, bcs2) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_BCS2));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_BCS2, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, bcs3) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_BCS3));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_BCS3, 0);
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

TEST_P(SimpleBatchBuffer, ccs1) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS1));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);
    mgr->setCCSMode(2);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_CCS1, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, ccs2) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS2));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);
    mgr->setCCSMode(4);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_CCS2, 0);
    addSimpleBatchBuffer(ctxt, GetParam());
    ctxt->pollForCompletion();
}

TEST_P(SimpleBatchBuffer, ccs3) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS3));

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);
    mgr->setCCSMode(4);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_CCS3, 0);
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

TEST_P(SimpleBatchBuffer, multiContext) {
    TEST_REQUIRES(gpu->isMemorySupported(GetParam(), 4096) || gpu->isMemorySupported(GetParam(), 65536));
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS3));

    bool rcsSupported = gpu->isEngineSupported(ENGINE_CCS3);

    desc.deviceCount = toMemoryBankId(static_cast<MemoryBank>(GetParam())) + 1;
    initializeStream(desc);
    mgr->setCCSMode(4);

    std::unique_ptr<HardwareContext> rcs;
    if (rcsSupported) {
        rcs = std::unique_ptr<HardwareContext>(mgr->createHardwareContext(defaultDevice, ENGINE_RCS, 0));
        addSimpleBatchBuffer(rcs.get(), GetParam());
    }

    auto ccs = std::unique_ptr<HardwareContext>(mgr->createHardwareContext(defaultDevice, ENGINE_CCS, 0));
    auto ccs1 = std::unique_ptr<HardwareContext>(mgr->createHardwareContext(defaultDevice, ENGINE_CCS1, 0));
    auto ccs2 = std::unique_ptr<HardwareContext>(mgr->createHardwareContext(defaultDevice, ENGINE_CCS2, 0));
    auto ccs3 = std::unique_ptr<HardwareContext>(mgr->createHardwareContext(defaultDevice, ENGINE_CCS3, 0));

    addSimpleBatchBuffer(ccs.get(), GetParam());
    addSimpleBatchBuffer(ccs1.get(), GetParam());
    addSimpleBatchBuffer(ccs2.get(), GetParam());
    addSimpleBatchBuffer(ccs3.get(), GetParam());

    if (rcsSupported) {
        rcs->pollForCompletion();
    }
    ccs->pollForCompletion();
    ccs1->pollForCompletion();
    ccs2->pollForCompletion();
    ccs3->pollForCompletion();
}

// This test submits 20 contexts where each tile has it's own process using 5 streams.
TEST_F(SimpleBatchBuffer, multiContextTile) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS3));
    TEST_REQUIRES(gpu->deviceCount > 3);

    desc.deviceCount = 4;

    initializeStream(desc);
    mgr->setCCSMode(4);

    auto device = 0;
    auto rcs_t0 = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    auto ccs_t0 = mgr->createHardwareContext(device, ENGINE_CCS, 0);
    auto ccs1_t0 = mgr->createHardwareContext(device, ENGINE_CCS1, 0);
    auto ccs2_t0 = mgr->createHardwareContext(device, ENGINE_CCS2, 0);
    auto ccs3_t0 = mgr->createHardwareContext(device, ENGINE_CCS3, 0);

    addSimpleBatchBuffer(rcs_t0, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs_t0, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs1_t0, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs2_t0, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs3_t0, MEMORY_BANK(device));

    device = 1;
    auto rcs_t1 = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    auto ccs_t1 = mgr->createHardwareContext(device, ENGINE_CCS, 0);
    auto ccs1_t1 = mgr->createHardwareContext(device, ENGINE_CCS1, 0);
    auto ccs2_t1 = mgr->createHardwareContext(device, ENGINE_CCS2, 0);
    auto ccs3_t1 = mgr->createHardwareContext(device, ENGINE_CCS3, 0);

    addSimpleBatchBuffer(rcs_t1, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs_t1, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs1_t1, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs2_t1, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs3_t1, MEMORY_BANK(device));

    device = 2;
    auto rcs_t2 = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    auto ccs_t2 = mgr->createHardwareContext(device, ENGINE_CCS, 0);
    auto ccs1_t2 = mgr->createHardwareContext(device, ENGINE_CCS1, 0);
    auto ccs2_t2 = mgr->createHardwareContext(device, ENGINE_CCS2, 0);
    auto ccs3_t2 = mgr->createHardwareContext(device, ENGINE_CCS3, 0);

    addSimpleBatchBuffer(rcs_t2, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs_t2, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs1_t2, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs2_t2, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs3_t2, MEMORY_BANK(device));

    device = 3;
    auto rcs_t3 = mgr->createHardwareContext(device, ENGINE_RCS, 0);
    auto ccs_t3 = mgr->createHardwareContext(device, ENGINE_CCS, 0);
    auto ccs1_t3 = mgr->createHardwareContext(device, ENGINE_CCS1, 0);
    auto ccs2_t3 = mgr->createHardwareContext(device, ENGINE_CCS2, 0);
    auto ccs3_t3 = mgr->createHardwareContext(device, ENGINE_CCS3, 0);

    addSimpleBatchBuffer(rcs_t3, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs_t3, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs1_t3, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs2_t3, MEMORY_BANK(device));
    addSimpleBatchBuffer(ccs3_t3, MEMORY_BANK(device));

    rcs_t0->pollForCompletion();
    ccs_t0->pollForCompletion();
    ccs1_t0->pollForCompletion();
    ccs2_t0->pollForCompletion();
    ccs3_t0->pollForCompletion();

    rcs_t1->pollForCompletion();
    ccs_t1->pollForCompletion();
    ccs1_t1->pollForCompletion();
    ccs2_t1->pollForCompletion();
    ccs3_t1->pollForCompletion();

    rcs_t2->pollForCompletion();
    ccs_t2->pollForCompletion();
    ccs1_t2->pollForCompletion();
    ccs2_t2->pollForCompletion();
    ccs3_t2->pollForCompletion();

    rcs_t3->pollForCompletion();
    ccs_t3->pollForCompletion();
    ccs1_t3->pollForCompletion();
    ccs2_t3->pollForCompletion();
    ccs3_t3->pollForCompletion();

    delete rcs_t0;
    delete ccs_t0;
    delete ccs1_t0;
    delete ccs2_t0;
    delete ccs3_t0;

    delete rcs_t1;
    delete ccs_t1;
    delete ccs1_t1;
    delete ccs2_t1;
    delete ccs3_t1;

    delete rcs_t2;
    delete ccs_t2;
    delete ccs1_t2;
    delete ccs2_t2;
    delete ccs3_t2;

    delete rcs_t3;
    delete ccs_t3;
    delete ccs1_t3;
    delete ccs2_t3;
    delete ccs3_t3;
}

HWTEST_F(SimpleBatchBuffer, submitSingleBatchBufferToMultipleTiles, MatchMultiDevice::moreThanOne) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));

    uintptr_t ppgttBatchBuffer = 0x8000003000;

    // Initialize batch buffer
    uint32_t batchCommands[] = {
        0x00000001,
        0x00000002,
        0x00000003,
        0x00000004,
        0x05000000,
    };

    desc.deviceCount = 4;

    initializeStream(desc);

    // All tiles refer to single physical storage
    mgr->writeMemory2({0x8000003000, batchCommands, sizeof(batchCommands), MemoryBank::MEMORY_BANK_0, DataTypeHintValues::TraceBatchBuffer, defaultPageSize});

    auto device = 0;
    auto rcs_t0 = static_cast<HardwareContextImp *>(mgr->createHardwareContext(device, ENGINE_RCS, 0));
    rcs_t0->initialize();
    rcs_t0->submitBatchBuffer(ppgttBatchBuffer, false);

    device = 1;
    auto rcs_t1 = static_cast<HardwareContextImp *>(mgr->createHardwareContext(device, ENGINE_RCS, 0));
    rcs_t1->initialize();
    rcs_t1->submitBatchBuffer(ppgttBatchBuffer, false);

    rcs_t0->pollForCompletion();
    rcs_t1->pollForCompletion();

    delete rcs_t0;
    delete rcs_t1;
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
