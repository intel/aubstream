/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"

#include "headers/aubstream.h"
#include "headers/engine_node.h"

#include "test_defaults.h"
#include "tests/unit_tests/hardware_context_tests.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "test.h"

#include <memory>

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST_F(HardwareContextTest, whenXeHpHardwareContextIsInitializedLRCAUses64KBPages) {
    TEST_REQUIRES(gpu->gfxCoreFamily == XE_HP_CORE);
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    auto context = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

    context->initialize();
    ASSERT_NE(nullptr, context);

    auto contextImp = static_cast<HardwareContextImp *>(context);
    EXPECT_EQ(0u, (contextImp->ggttLRCA & 0xffff));

    delete context;
}

TEST_F(HardwareContextTest, whenXeHpHardwareContextIsInitializedRingBufferUses64KBPages) {
    TEST_REQUIRES(gpu->gfxCoreFamily == XE_HP_CORE);
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    auto context = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

    context->initialize();
    ASSERT_NE(nullptr, context);

    auto contextImp = static_cast<HardwareContextImp *>(context);
    EXPECT_EQ(0u, (contextImp->ggttRing & 0xffff));

    delete context;
}

TEST_F(HardwareContextTest, whenXeHpHardwareContextIsInitializedGlobalHWSPUses64KBPages) {
    TEST_REQUIRES(gpu->gfxCoreFamily == XE_HP_CORE);
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    auto context = aubManager.createHardwareContext(0, ENGINE_RCS, 0);
    auto contextImp = static_cast<HardwareContextImp *>(context);

    // Force an unaligned first page
    contextImp->ggtt.gfxAddressAllocator.alignedAlloc(0x1000, 0x10000);

    context->initialize();
    ASSERT_NE(nullptr, context);

    EXPECT_EQ(0u, (contextImp->ggttGlobalHWSP & 0xffff));

    delete context;
}

TEST_F(HardwareContextTest, XeHpGiven4KBLocalPageWriteMemoryPromotesTo64KB) {
    TEST_REQUIRES(gpu->gfxCoreFamily == XE_HP_CORE);
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x0000;

    auto &csHelper = getCommandStreamerHelper(gpu->productFamily, defaultDevice, ENGINE_RCS);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);
    context.writeMemory2({gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, 4096});

    auto pdp = ppgtt.getChild(0);
    ASSERT_NE(nullptr, pdp);

    auto pde = pdp->getChild(0);
    ASSERT_NE(nullptr, pde);

    auto pte = static_cast<PTE *>(pde->getChild(0));
    ASSERT_NE(nullptr, pte);
    EXPECT_EQ(65536, pte->getPageSize());
}
