/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/misc_helpers.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/settings.h"

#include "aubstream/aubstream.h"
#include "aubstream/engine_node.h"

#include "hardware_context_tests.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/variable_backup.h"
#include "test.h"
#include "aub_mem_dump/memcpy_s.h"
#include <memory>

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST_F(HardwareContextTest, whenHardwareContextIsCreatedThenItHasCorrectDeviceIndexSet) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(2, stream, csHelper, ggtt, ppgtt, 0);
    EXPECT_EQ(2u, context.deviceIndex);
}

TEST_F(HardwareContextTest, whenHardwareContextIsInitializedThenItAllocatesLRCA) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.initialize();
    auto context = aubManager.createHardwareContext(0, ENGINE_CCS, 0);

    context->initialize();
    ASSERT_NE(nullptr, context);

    EXPECT_NE(nullptr, static_cast<HardwareContextImp *>(context)->pLRCA);

    delete context;
}

TEST_F(HardwareContextTest, whenHardwareContextIsInitializedTwiceThenItDoesntReallocatesLRCA) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.initialize();
    auto context = aubManager.createHardwareContext(0, ENGINE_CCS, 0);

    context->initialize();
    ASSERT_NE(nullptr, context);
    auto pLRCA1 = static_cast<HardwareContextImp *>(context)->pLRCA;
    EXPECT_NE(nullptr, pLRCA1);

    context->initialize();
    ASSERT_NE(nullptr, context);
    auto pLRCA2 = static_cast<HardwareContextImp *>(context)->pLRCA;
    EXPECT_NE(nullptr, pLRCA2);

    EXPECT_EQ(pLRCA1, pLRCA2);

    delete context;
}

static bool ringDataDisabled(const aub_stream::Gpu *gpu) {
    return !gpu->getCommandStreamerHelper(defaultDevice, defaultEngine).isRingDataEnabled();
}

HWTEST_F(HardwareContextTest, ringBufferWrap, ringDataDisabled) {
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);

    context.initialize();

    auto ppgttBatchBuffer = 0x10000;
    uint32_t data = 0xbaddf00d;

    // Do an initial submission to determine original submission size
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, _)).Times(3);
    context.writeAndSubmitBatchBuffer(ppgttBatchBuffer, &data, sizeof(data), defaultMemoryBank, defaultPageSize);
    auto ringTailAfterFirstSubmission = context.ringTail;

    // Force the context into a ring buffer wrap
    context.ringTail = uint32_t(context.ringSize - sizeof(uint32_t));
    auto ringTailAlmostFull = context.ringTail;

    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, _)).Times(4); // + 1 for nooping write
    context.writeAndSubmitBatchBuffer(ppgttBatchBuffer, &data, sizeof(data), defaultMemoryBank, defaultPageSize);

    EXPECT_LT(context.ringTail, ringTailAlmostFull);
    EXPECT_EQ(ringTailAfterFirstSubmission, context.ringTail);
}

TEST_F(HardwareContextTest, pollForCompletionShouldForwardToRegisterPoll) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);

    EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(AtLeast(1));
    context.pollForCompletion();
}

TEST_F(HardwareContextTest, submitShouldPerformAtLeastOneMMIOWrite) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(1));
    SimpleAllocator<uint64_t> gfxAddressAllocator(0x1000);
    uintptr_t ppgttBatchBuffer = gfxAddressAllocator.alignedAlloc(0x1000, uint32_t(defaultPageSize));
    uint32_t data = 0x05000000;
    context.writeAndSubmitBatchBuffer(ppgttBatchBuffer, &data, sizeof(data), defaultMemoryBank, defaultPageSize);
}

TEST_F(HardwareContextTest, submitShouldPerformFenceOperations) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(1));
    SimpleAllocator<uint64_t> gfxAddressAllocator(0x1000);
    uintptr_t ppgttBatchBuffer = gfxAddressAllocator.alignedAlloc(0x2000, uint32_t(defaultPageSize));
    EXPECT_EQ(context.getExpectedFence(), 0);
    context.submitBatchBuffer(ppgttBatchBuffer, true);
    EXPECT_EQ(context.getExpectedFence(), 1);
    context.submitBatchBuffer(ppgttBatchBuffer, true);
    EXPECT_EQ(context.getExpectedFence(), 2);
    context.submitBatchBuffer(ppgttBatchBuffer, true);
    EXPECT_EQ(context.getExpectedFence(), 3);
    EXPECT_CALL(stream, readDiscontiguousPages(_, _, _)).Times(AtLeast(1));
    context.getCurrentFence();
}

HWTEST_F(HardwareContextTest, submitBatchBufferShouldPerformAtLeastOneMMIOWriteAndDiscontiguousPageWrites, ringDataDisabled) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    SimpleAllocator<uint64_t> gfxAddressAllocator(0x1000);
    uintptr_t ppgttBatchBuffer = gfxAddressAllocator.alignedAlloc(0x1000, uint32_t(defaultPageSize));
    uint32_t data = 0x05000000;

    // 4 PPGTT levels
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _)).Times(4);
    // batchBuffer contents write
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, _)).Times(1);
    context.writeMemory2({ppgttBatchBuffer, &data, sizeof(data), defaultMemoryBank, context.csTraits.aubHintBatchBuffer, defaultPageSize});

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(1));
    // two GGTT updates
    auto numGGTTUpdates = 2;
    EXPECT_CALL(stream, writeGttPages(_, _)).Times(numGGTTUpdates);

    // ring commands and LRCA writes
    auto numWrites = 2;
    EXPECT_CALL(stream, writeDiscontiguousPages(_, _, _, _)).Times(numWrites);
    context.submitBatchBuffer(ppgttBatchBuffer, false);
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingFreeMemoryThenEntriesAreRemovedFromPageTable) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();
    context.writeMemory2({gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, 0, defaultPageSize});

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    if (defaultMemoryBank == MEMORY_BANK_SYSTEM) {
        EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TracePpgttPdEntry, DataTypeHintValues::TraceNotype)).Times(1);
        EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype)).Times(1);
    } else {
        EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2)).Times(1);
        EXPECT_CALL(stream, writeDiscontiguousPages(_, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1)).Times(1);
    }

    context.freeMemory(gfxAddress, sizeof(bytes));
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingFreeMemoryOnNonExistingAddressThenDontCrash) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0x1000;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    context.freeMemory(gfxAddress, sizeof(bytes));
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingExpectMemoryRedirectsToAubStream) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;
    uint32_t compareOperation = 0;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();
    context.writeMemory2({gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, 0, defaultPageSize});

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, expectMemoryTable(bytes, sizeof(bytes), _, compareOperation)).Times(1);

    context.expectMemory(gfxAddress, bytes, sizeof(bytes), compareOperation);
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingReadMemoryRedirectsToAubStream) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();
    context.writeMemory2({gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, 0, defaultPageSize});

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    ::testing::InSequence inSequence;
    std::vector<PageInfo> pageTableEntries;
    EXPECT_CALL(stream, readDiscontiguousPages(bytes, sizeof(bytes), _))
        .Times(1)
        .WillOnce(::testing::Invoke([&](void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) { pageTableEntries = writeInfoTable; }));

    context.readMemory(gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, defaultPageSize);
    ASSERT_EQ(1u, pageTableEntries.size());
    EXPECT_EQ(defaultMemoryBank, pageTableEntries[0].memoryBank);
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingWriteMMIORedirectsToAubStream) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    uint32_t offset = 0x01234567;
    uint32_t value = 0x89ABCDEF;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    EXPECT_CALL(stream, writeMMIO(offset, value)).Times(1);

    context.writeMMIO(offset, value);

    ::testing::Mock::VerifyAndClearExpectations(&stream);
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingDumpBufferBINRedirectsToAubStream) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto gfxAddress = 0x1000;
    auto size = 100;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress, size, _)).Times(1);

    context.dumpBufferBIN(gfxAddress, size);
}

TEST_F(HardwareContextTest, givenHardwareContextWhenCallingDumpBufferRedirectsToAubStream) {
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto gfxAddress = 0x1000;
    auto size = 100;

    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(1, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, dumpSurface(_, _, _)).Times(1);

    SurfaceInfo surfaceInfo;
    surfaceInfo.address = gfxAddress;
    surfaceInfo.width = size;
    surfaceInfo.height = 1;
    surfaceInfo.pitch = size;
    surfaceInfo.format = 0x1ff;
    surfaceInfo.tilingType = tilingType::linear;
    surfaceInfo.surftype = surftype::buffer;
    surfaceInfo.compressed = false;
    surfaceInfo.dumpType = dumpType::tre;
    context.dumpSurface(surfaceInfo);
}

TEST_F(HardwareContextTest, pollForFenceCompletionShouldForwardToReadMemory) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);

    EXPECT_CALL(stream, readDiscontiguousPages(_, _, _)).Times(1);
    context.pollForFenceCompletion();
}

TEST_F(HardwareContextTest, pollForFenceCompletionShouldForwardToReadMemoryUntilExpectedValue) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);
    context.contextFenceValue = 17;
    uint32_t v = 0;
    EXPECT_CALL(stream, readDiscontiguousPages(_, _, _)).Times(18).WillRepeatedly([&v](void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) {
        memcpy_s(memory, size, &v, sizeof(v));
        v++;
    });
    context.pollForFenceCompletion();
}

TEST_F(HardwareContextTest, checkContextIdIsUnique) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    // Depending which tests ran before this we will have created contexts for those,
    // thus take note of where we are starting  from
    const uint32_t contextIdBase = HardwareContextImp::globalContextId;
    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, 0);
    HardwareContextImp context1(0, stream, csHelper, ggtt, ppgtt, 0);
    HardwareContextImp context2(0, stream, csHelper, ggtt, ppgtt, 0);
    HardwareContextImp context3(0, stream, csHelper, ggtt, ppgtt, 0);

    EXPECT_EQ(context0.contextId, contextIdBase + 0);
    EXPECT_EQ(context1.contextId, contextIdBase + 1);
    EXPECT_EQ(context2.contextId, contextIdBase + 2);
    EXPECT_EQ(context3.contextId, contextIdBase + 3);
}

TEST_F(HardwareContextTest, givenGroupContextWhenSubmittingThenGroupAsSingleExeclist) {
    TEST_REQUIRES(gpu->gfxCoreFamily >= CoreFamily::Gen12lp && gpu->gfxCoreFamily <= CoreFamily::XeHpcCore);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    GroupContextHelper helper;

    // Initialize contextGroups contexts
    auto contextGroupCount = gpu->getContextGroupCount();
    for (auto i = 0u; i < arrayCount(helper.contextGroups); i++) {
        for (auto j = 0u; j < arrayCount(helper.contextGroups[i]); j++) {
            helper.contextGroups[i][j].resize(1);
            helper.contextGroups[i][j][0].contexts.resize(contextGroupCount);
        }
    }

    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[defaultDevice][defaultEngine][0], hardwareContextFlags::contextGroup);
    context0.initialize();
    HardwareContextImp context1(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[defaultDevice][defaultEngine][0], hardwareContextFlags::contextGroup);
    context1.initialize();
    HardwareContextImp context2(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[defaultDevice][defaultEngine][0], hardwareContextFlags::contextGroup);
    context2.initialize();
    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(1));

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510, _)).Times(1);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514, _)).Times(1);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510 + 8, _)).Times(1);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514 + 8, _)).Times(1);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510 + 16, _)).Times(1);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514 + 16, _)).Times(1);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2550, _)).Times(1);
    context0.submitBatchBuffer(0x100, false);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510, _)).Times(1);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514, _)).Times(1);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510 + 8, _)).Times(1);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514 + 8, _)).Times(1);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510 + 16, _)).Times(1);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514 + 16, _)).Times(1);

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2550, _)).Times(1);

    context2.submitBatchBuffer(0x100, false);
}

TEST_F(HardwareContextTest, givenGroupContextWhenMainHardwareContextDestroyedThenGroupIsFreed) {
    TEST_REQUIRES(gpu->gfxCoreFamily >= CoreFamily::XeHpcCore);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    GroupContextHelper helper;

    // Initialize contextGroups contexts
    auto contextGroupCount = gpu->getContextGroupCount();
    for (auto i = 0u; i < arrayCount(helper.contextGroups); i++) {
        for (auto j = 0u; j < arrayCount(helper.contextGroups[i]); j++) {
            helper.contextGroups[i][j].resize(1);
            helper.contextGroups[i][j][0].contexts.resize(contextGroupCount);
        }
    }

    {
        auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[defaultDevice][defaultEngine][0], hardwareContextFlags::contextGroup);
        context0->initialize();
        auto context1 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[defaultDevice][defaultEngine][0], hardwareContextFlags::contextGroup);
        context1->initialize();
        auto context2 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[defaultDevice][defaultEngine][0], hardwareContextFlags::contextGroup);
        context2->initialize();

        ::testing::Mock::VerifyAndClearExpectations(&stream);

        context0->submitBatchBuffer(0x100, false);

        context2->submitBatchBuffer(0x100, false);

        EXPECT_EQ(3u, helper.contextGroups[defaultDevice][csHelper.engineType][0].contextGroupCounter);
        EXPECT_NE(nullptr, helper.contextGroups[defaultDevice][csHelper.engineType][0].contexts[0]);
        EXPECT_NE(nullptr, helper.contextGroups[defaultDevice][csHelper.engineType][0].contexts[2]);
    }

    EXPECT_EQ(0u, helper.contextGroups[defaultDevice][csHelper.engineType][0].contextGroupCounter);
    EXPECT_EQ(nullptr, helper.contextGroups[defaultDevice][csHelper.engineType][0].contexts[0]);
    EXPECT_EQ(nullptr, helper.contextGroups[defaultDevice][csHelper.engineType][0].contexts[2]);
}

TEST_F(HardwareContextTest, givenMultipleContextGroupsWhenHardwareContextsCreatedThenCorrectGroupIsAssigned) {
    TEST_REQUIRES(gpu->gfxCoreFamily >= CoreFamily::XeHpcCore);

    auto gpu = createGpuFunc();
    auto gpuPtr = gpu.get();
    MockAubManager aubManager(std::move(gpu), 1, defaultHBMSizePerDevice, 0u, true, aub_stream::mode::aubFile);
    aubManager.initialize();

    auto &csHelper = gpuPtr->getCommandStreamerHelper(defaultDevice, defaultEngine);

    CreateHardwareContext2Params params = {
        0,
        hardwareContextId::invalidContextId};

    auto context0 = aubManager.createHardwareContext2(params, defaultDevice, defaultEngine, hardwareContextFlags::contextGroup);
    context0->initialize();

    EXPECT_EQ(1u, aubManager.getGroupContextHelper()->groupIds[defaultDevice][csHelper.engineType]);
    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->primaryContextIdToGroupId[defaultDevice][csHelper.engineType][0]);

    CreateHardwareContext2Params params1 = {
        1,
        0};
    auto context1 = aubManager.createHardwareContext2(params1, defaultDevice, defaultEngine, hardwareContextFlags::contextGroup);
    context1->initialize();

    CreateHardwareContext2Params params2 = {
        2,
        0};
    auto context2 = aubManager.createHardwareContext2(params2, defaultDevice, defaultEngine, hardwareContextFlags::contextGroup);
    context2->initialize();

    EXPECT_EQ(3u, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contextGroupCounter);
    EXPECT_EQ(1u, aubManager.getGroupContextHelper()->groupIds[defaultDevice][csHelper.engineType]);
    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->primaryContextIdToGroupId[defaultDevice][csHelper.engineType][0]);

    EXPECT_EQ(context0, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contexts[0]);
    EXPECT_EQ(context1, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contexts[1]);
    EXPECT_EQ(context2, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contexts[2]);

    CreateHardwareContext2Params params20 = {
        20,
        hardwareContextId::invalidContextId};
    auto context20 = aubManager.createHardwareContext2(params20, defaultDevice, defaultEngine, hardwareContextFlags::contextGroup);
    context20->initialize();

    EXPECT_EQ(2u, aubManager.getGroupContextHelper()->groupIds[defaultDevice][csHelper.engineType]);
    EXPECT_EQ(1u, aubManager.getGroupContextHelper()->primaryContextIdToGroupId[defaultDevice][csHelper.engineType][20]);

    CreateHardwareContext2Params params21 = {
        21,
        20};
    auto context21 = aubManager.createHardwareContext2(params21, defaultDevice, defaultEngine, hardwareContextFlags::contextGroup);
    context21->initialize();

    EXPECT_EQ(2u, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][1].contextGroupCounter);
    EXPECT_EQ(2u, aubManager.getGroupContextHelper()->groupIds[defaultDevice][csHelper.engineType]);
    EXPECT_EQ(1u, aubManager.getGroupContextHelper()->primaryContextIdToGroupId[defaultDevice][csHelper.engineType][20]);

    EXPECT_EQ(context20, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][1].contexts[0]);
    EXPECT_EQ(context21, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][1].contexts[1]);

    aubManager.releaseHardwareContext(context0);
    aubManager.releaseHardwareContext(context1);
    aubManager.releaseHardwareContext(context2);
    aubManager.releaseHardwareContext(context20);
    aubManager.releaseHardwareContext(context21);

    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contextGroupCounter);

    EXPECT_EQ(nullptr, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contexts[0]);
    EXPECT_EQ(nullptr, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contexts[1]);
    EXPECT_EQ(nullptr, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contexts[2]);

    EXPECT_EQ(nullptr, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][1].contexts[0]);
    EXPECT_EQ(nullptr, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][1].contexts[1]);
}

TEST_F(HardwareContextTest, givenNoContextGroupFlagWhenHardwareContextCreatedThenGroupIsNotAssigned) {
    TEST_REQUIRES(gpu->gfxCoreFamily >= CoreFamily::XeHpcCore);

    auto gpu = createGpuFunc();
    auto gpuPtr = gpu.get();
    MockAubManager aubManager(std::move(gpu), 1, defaultHBMSizePerDevice, 0u, true, aub_stream::mode::aubFile);
    aubManager.initialize();

    auto &csHelper = gpuPtr->getCommandStreamerHelper(defaultDevice, defaultEngine);

    CreateHardwareContext2Params params = {
        0,
        hardwareContextId::invalidContextId};

    auto context0 = aubManager.createHardwareContext2(params, defaultDevice, defaultEngine, 0);
    context0->initialize();

    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->groupIds[defaultDevice][csHelper.engineType]);
    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->primaryContextIdToGroupId[defaultDevice][csHelper.engineType].size());

    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->contextGroups[defaultDevice][csHelper.engineType][0].contextGroupCounter);
    EXPECT_EQ(0u, aubManager.getGroupContextHelper()->groupIds[defaultDevice][csHelper.engineType]);
    EXPECT_TRUE(aubManager.getGroupContextHelper()->primaryContextIdToGroupId[defaultDevice][csHelper.engineType].empty());
    aubManager.releaseHardwareContext(context0);
}

TEST_F(HardwareContextTest, givenPriorityWhenCreateHardwareContext3CalledThenPriorityIsSet) {
    auto gpu = createGpuFunc();
    MockAubManager aubManager(std::move(gpu), 1, defaultHBMSizePerDevice, 0u, true, aub_stream::mode::aubFile);
    aubManager.initialize();

    CreateHardwareContext3Params params = {};
    params.header.size = sizeof(CreateHardwareContext3Params);
    params.device = defaultDevice;
    params.engine = defaultEngine;
    params.flags = 0;

    {
        auto context = aubManager.createHardwareContext3(&params.header);
        context->initialize();
        EXPECT_EQ(HardwareContextImp::priorityLow, static_cast<HardwareContextImp *>(context)->priority);

        aubManager.releaseHardwareContext(context);
    }

    {
        params.priority = HardwareContextImp::priorityHigh;
        auto context0 = aubManager.createHardwareContext3(&params.header);
        context0->initialize();
        EXPECT_EQ(HardwareContextImp::priorityHigh, static_cast<HardwareContextImp *>(context0)->priority);

        aubManager.releaseHardwareContext(context0);
    }

    {
        params.priority = HardwareContextImp::priorityNormal;
        auto context1 = aubManager.createHardwareContext3(&params.header);
        context1->initialize();
        EXPECT_EQ(HardwareContextImp::priorityNormal, static_cast<HardwareContextImp *>(context1)->priority);

        aubManager.releaseHardwareContext(context1);
    }
}

TEST_F(HardwareContextTest, givenIncorrectVersionOrSizeWhenCreateHardwareContext3CalledThenNullptrReturned) {
    auto gpu = createGpuFunc();
    MockAubManager aubManager(std::move(gpu), 1, defaultHBMSizePerDevice, 0u, true, aub_stream::mode::aubFile);
    aubManager.initialize();

    CreateHardwareContext3Params params = {};

    params.header.version = 5;
    params.header.size = sizeof(CreateHardwareContext3Params);
    params.device = defaultDevice;
    params.engine = defaultEngine;
    params.flags = 0;

    auto context = aubManager.createHardwareContext3(&params.header);
    EXPECT_EQ(nullptr, context);

    params.header.version = 1;
    params.header.size = sizeof(CreateHardwareContext3Params) + 4;

    context = aubManager.createHardwareContext3(&params.header);
    EXPECT_EQ(nullptr, context);
}

HWTEST_F(HardwareContextTest, givenHighPriorityFlagWhenSubmittingHardwareContextThenContextDescriptorHasCorrectBitsSet, HwMatcher::coreEqualGen12Core) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, hardwareContextFlags::highPriority);
    context0->initialize();

    EXPECT_EQ(HardwareContextImp::priorityHigh, context0->priority);

    MiContextDescriptorReg contextDescriptor = {};

    contextDescriptor.sData.Valid = true;
    contextDescriptor.sData.ForcePageDirRestore = false;
    contextDescriptor.sData.ForceRestore = false;
    contextDescriptor.sData.Legacy = true;
    contextDescriptor.sData.FaultSupport = 0;
    contextDescriptor.sData.PrivilegeAccessOrPPGTT = true;
    contextDescriptor.sData.ADor64bitSupport = ppgtt.getNumAddressBits() != 32;

    contextDescriptor.sData.LogicalRingCtxAddress = context0->ggttLRCA / 4096;
    contextDescriptor.sData.Reserved = 0;
    contextDescriptor.sData.ContextID = context0->contextId;
    contextDescriptor.sData.Reserved2 = 0;

    auto value = contextDescriptor.ulData[0];

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510, value));

    context0->submitBatchBuffer(0x100, false);
}
HWTEST_F(HardwareContextTest, givenLowPriorityFlagWhenSubmittingHardwareContextThenContextDescriptorHasCorrectBitsSet, HwMatcher::coreEqualGen12Core) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, hardwareContextFlags::lowPriority);
    context0->initialize();

    EXPECT_EQ(HardwareContextImp::priorityLow, context0->priority);

    MiContextDescriptorReg contextDescriptor = {};

    contextDescriptor.sData.Valid = true;
    contextDescriptor.sData.ForcePageDirRestore = false;
    contextDescriptor.sData.ForceRestore = false;
    contextDescriptor.sData.Legacy = true;
    contextDescriptor.sData.FaultSupport = 0;
    contextDescriptor.sData.PrivilegeAccessOrPPGTT = true;
    contextDescriptor.sData.ADor64bitSupport = ppgtt.getNumAddressBits() != 32;

    contextDescriptor.sData.LogicalRingCtxAddress = context0->ggttLRCA / 4096;
    contextDescriptor.sData.Reserved = 0;
    contextDescriptor.sData.ContextID = context0->contextId;
    contextDescriptor.sData.Reserved2 = 0;

    auto value = contextDescriptor.ulData[0];

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510, value));

    context0->submitBatchBuffer(0x100, false);
}

TEST_F(HardwareContextTest, givenHighPriorityFlagWhenSubmittingHardwareContextThenContextDescriptorHasPriorityBitSet) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpcCore);
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, hardwareContextFlags::highPriority);
    context0->initialize();

    MiContextDescriptorReg contextDescriptor = {};

    contextDescriptor.sData.Valid = true;
    contextDescriptor.sData.ForcePageDirRestore = false;
    contextDescriptor.sData.ForceRestore = false;
    contextDescriptor.sData.Legacy = true;
    contextDescriptor.sData.FaultSupport = 0;
    contextDescriptor.sData.PrivilegeAccessOrPPGTT = true;
    contextDescriptor.sData.ADor64bitSupport = ppgtt.getNumAddressBits() != 32;

    contextDescriptor.sData.LogicalRingCtxAddress = context0->ggttLRCA / 4096;
    contextDescriptor.sData.Reserved = 0;
    contextDescriptor.sData.ContextID = context0->contextId;
    contextDescriptor.sData.Reserved2 = 0;

    contextDescriptor.sData.FunctionType = 2;
    auto value = contextDescriptor.ulData[0];

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510, value));

    context0->submitBatchBuffer(0x100, false);
}

TEST_F(HardwareContextTest, givenLowPriorityFlagWhenSubmittingHardwareContextThenContextDescriptorHasPriorityBitSet) {
    TEST_REQUIRES(gpu->gfxCoreFamily == CoreFamily::XeHpcCore);
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, hardwareContextFlags::lowPriority);
    context0->initialize();

    MiContextDescriptorReg contextDescriptor = {};

    contextDescriptor.sData.Valid = true;
    contextDescriptor.sData.ForcePageDirRestore = false;
    contextDescriptor.sData.ForceRestore = false;
    contextDescriptor.sData.Legacy = true;
    contextDescriptor.sData.FaultSupport = 0;
    contextDescriptor.sData.PrivilegeAccessOrPPGTT = true;
    contextDescriptor.sData.ADor64bitSupport = ppgtt.getNumAddressBits() != 32;

    contextDescriptor.sData.LogicalRingCtxAddress = context0->ggttLRCA / 4096;
    contextDescriptor.sData.Reserved = 0;
    contextDescriptor.sData.ContextID = context0->contextId;
    contextDescriptor.sData.Reserved2 = 0;

    contextDescriptor.sData.FunctionType = 0;
    auto value = contextDescriptor.ulData[0];

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510, value));

    context0->submitBatchBuffer(0x100, false);
}

TEST_F(HardwareContextTest, givenRunAloneFlagWhenInitializingHardwareContextThenContextSaveRestoreHasCorrectBitsSet) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, hardwareContextFlags::runAlone);
    uint32_t value = csHelper.getInitialContextSaveRestoreCtrlValue();

    value |= 0x800080;
    context0->initialize();

    auto pLRI = context0->pLRCA;
    pLRI = &pLRI[context0->csTraits.offsetLRI0 + context0->csTraits.offsetContext + 4];

    uint32_t *lri = reinterpret_cast<uint32_t *>(pLRI);
    EXPECT_EQ(*lri, csHelper.mmioEngine + 0x2244);
    lri = lri + 1;
    EXPECT_EQ(*lri, value);
}

TEST_F(HardwareContextTest, givenVerboseLogLevelWhenSubmittingContextThenRingHeadTailIsPrinted) {
    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->LogLevel.set(LogLevels::verbose);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);
    context.initialize();

    // EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(1));
    SimpleAllocator<uint64_t> gfxAddressAllocator(0x1000);
    uintptr_t ppgttBatchBuffer = gfxAddressAllocator.alignedAlloc(0x1000, uint32_t(defaultPageSize));
    uint32_t data = 0x05000000;

    ::testing::internal::CaptureStdout();
    context.writeAndSubmitBatchBuffer(ppgttBatchBuffer, &data, sizeof(data), defaultMemoryBank, defaultPageSize);

    std::string output = testing::internal::GetCapturedStdout();

    std::string expectedString = "[VERBOSE]  contextId = ";

    expectedString += std::to_string(context.contextId) + " \n";
    expectedString += "[VERBOSE]  ringHead = 0 \n";
    expectedString += "[VERBOSE]  ringTail = " + std::to_string(context.ringTail) + " \n";

    EXPECT_STREQ(expectedString.c_str(), output.c_str());
}

HWTEST_F(HardwareContextTest, givenContextWhenSubmittingThenUseExeclistPortSubmission, HwMatcher::coreEqualGreaterXe3p) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    GroupContextHelper helper;

    // Initialize contextGroups contexts
    auto contextGroupCount = gpu->getContextGroupCount();
    for (auto i = 0u; i < arrayCount(helper.contextGroups); i++) {
        for (auto j = 0u; j < arrayCount(helper.contextGroups[i]); j++) {
            helper.contextGroups[i][j].resize(1);
            helper.contextGroups[i][j][0].contexts.resize(contextGroupCount);
        }
    }

    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, &helper.contextGroups[0][EngineType::ENGINE_CCS][0], (1 << 15));
    context0.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AnyNumber());

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2230, _)).Times(2 * contextGroupCount);
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2550, 1)).Times(1);

    context0.submitBatchBuffer(0x100, false);
}

HWTEST_F(HardwareContextTest, givenExeclistSubmitPortSubmissionDisabledWhenSubmittingThenSQSubmissionIsUsed, HwMatcher::coreEqualGreaterXe3p) {
    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->ExeclistSubmitPortSubmission.set(0);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, 0);
    context0.initialize();

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AnyNumber());

    for (int i = 0; i < 8; i++) {
        EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2510 + (i * 8), _)).Times(1);
        EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2514 + (i * 8), _)).Times(1);
    }
    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2550, 1)).Times(1);

    context0.submitBatchBuffer(0x100, false);
}

HWTEST_F(HardwareContextTest, givenXe3pHighPriorityFlagWhenSubmittingHardwareContextThenContextDescriptorHasPriorityBitSet, HwMatcher::coreEqualGreaterXe3p) {
    auto settings = std::make_unique<Settings>();
    VariableBackup<Settings *> backup(&globalSettings);
    globalSettings = settings.get();
    globalSettings->ExeclistSubmitPortSubmission.set(1);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, hardwareContextFlags::highPriority);
    context0->initialize();

    MiContextDescriptorReg contextDescriptor = {};

    contextDescriptor.sData.Valid = true;
    contextDescriptor.sData.ForcePageDirRestore = false;
    contextDescriptor.sData.ForceRestore = false;
    contextDescriptor.sData.Legacy = true;
    contextDescriptor.sData.FaultSupport = 0;
    contextDescriptor.sData.PrivilegeAccessOrPPGTT = true;
    contextDescriptor.sData.ADor64bitSupport = ppgtt.getNumAddressBits() != 32;

    contextDescriptor.sData.LogicalRingCtxAddress = context0->ggttLRCA / 4096;
    contextDescriptor.sData.Reserved = 0;
    contextDescriptor.sData.ContextID = context0->contextId;
    contextDescriptor.sData.Reserved2 = 0;

    contextDescriptor.sData.FunctionType = HardwareContextImp::priorityHigh;
    auto value = contextDescriptor.ulData[0];

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(::testing::AtLeast(0));

    EXPECT_CALL(stream, writeMMIO(csHelper.mmioEngine + 0x2230, value));
    context0->submitBatchBuffer(0x100, false);
}

struct MockCommandStreamerHelperWithMemoryPoll : public CommandStreamerHelperCcs {
    MockCommandStreamerHelperWithMemoryPoll(uint32_t deviceId, uint32_t engineId) : CommandStreamerHelperCcs(deviceId, engineId) {}

    bool memoryBasedPollForCompletion() const override { return true; }

    const MMIOList getEngineMMIO() const override { return {}; }

  protected:
    void submitContext(AubStream &stream, std::vector<MiContextDescriptorReg> &contextDescriptor) const override {}
};

using ComparisonValues = CmdServicesMemTraceMemoryPoll::ComparisonValues;

TEST_F(HardwareContextTest, givenMemoryBasedPollForCompletionWhenPollForCompletionIsCalledThenMemoryPollIsUsed) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);

    MockCommandStreamerHelperWithMemoryPoll csHelper(defaultDevice, 0);
    csHelper.gpu = gpu.get();
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);

    EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(0);
    EXPECT_CALL(stream, gttMemoryPoll(_, _, _, ComparisonValues::GreaterEqual)).Times(1);

    context.pollForCompletion();
}

TEST_F(HardwareContextTest, givenRegisterBasedPollForCompletionWhenPollForCompletionIsCalledThenRegisterPollIsUsed) {
    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);
    HardwareContextImp context(0, stream, csHelper, ggtt, ppgtt, 0);

    if (csHelper.memoryBasedPollForCompletion()) {
        EXPECT_CALL(stream, memoryPoll(_, _, _)).Times(1);
        EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(0);
    } else {
        EXPECT_CALL(stream, memoryPoll(_, _, _)).Times(0);
        EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(AtLeast(1));
    }

    context.pollForCompletion();
}
