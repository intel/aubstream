/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"

#include "aubstream/aubstream.h"
#include "aubstream/engine_node.h"

#include "hardware_context_tests.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "test.h"
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
    auto context = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

    context->initialize();
    ASSERT_NE(nullptr, context);

    EXPECT_NE(nullptr, static_cast<HardwareContextImp *>(context)->pLRCA);

    delete context;
}

TEST_F(HardwareContextTest, whenHardwareContextIsInitializedTwiceThenItDoesntReallocatesLRCA) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.initialize();
    auto context = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

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

    EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(1);
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

    ::testing::Mock::VerifyAndClearExpectations(&stream);

    EXPECT_CALL(stream, writeMMIO(offset, value)).Times(1);

    context.writeMMIO(offset, value);
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

    HardwareContextImp context0(0, stream, csHelper, ggtt, ppgtt, (1 << 15));
    context0.initialize();
    HardwareContextImp context1(0, stream, csHelper, ggtt, ppgtt, (1 << 15));
    context1.initialize();
    HardwareContextImp context2(0, stream, csHelper, ggtt, ppgtt, (1 << 15));
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

    HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contextGroupCounter = 0;
    HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contexts = {};
}

TEST_F(HardwareContextTest, givenGroupContextWhenMainHardwareContextDestroyedThenGroupIsFreed) {
    TEST_REQUIRES(gpu->gfxCoreFamily >= CoreFamily::XeHpcCore);

    PhysicalAddressAllocatorSimple allocator;
    GGTT ggtt(*gpu, &allocator, defaultMemoryBank);
    PML4 ppgtt(*gpu, &allocator, defaultMemoryBank);
    auto &csHelper = gpu->getCommandStreamerHelper(defaultDevice, defaultEngine);

    {
        auto context0 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, (1 << 15));
        context0->initialize();
        auto context1 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, (1 << 15));
        context1->initialize();
        auto context2 = std::make_unique<HardwareContextImp>(0, stream, csHelper, ggtt, ppgtt, (1 << 15));
        context2->initialize();

        ::testing::Mock::VerifyAndClearExpectations(&stream);

        context0->submitBatchBuffer(0x100, false);

        context2->submitBatchBuffer(0x100, false);

        EXPECT_EQ(3u, HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contextGroupCounter);
        EXPECT_NE(nullptr, HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contexts[0]);
        EXPECT_NE(nullptr, HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contexts[2]);
    }

    EXPECT_EQ(0u, HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contextGroupCounter);
    EXPECT_EQ(nullptr, HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contexts[0]);
    EXPECT_EQ(nullptr, HardwareContextImp::contextGroups[defaultDevice][csHelper.engineType].contexts[2]);
}
