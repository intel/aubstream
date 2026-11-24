/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gpu.h"

#include "aubstream/hardware_context.h"
#include "test_defaults.h"
#include "tests/unit_tests/command_stream_helper_tests.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include "test.h"

#include <vector>

using namespace aub_stream;
using ::testing::_;
using ::testing::Return;

namespace {
constexpr uint32_t tlbInvalidationDescriptorDword0 = 0x0CF7C;
constexpr uint32_t tlbInvalidationDescriptorDword1 = 0x0CF80;
constexpr uint32_t invalidateAllMappingsDescriptorDword0 = 0x1;
constexpr uint32_t maxAckStatusReads = 11;
} // namespace

using TlbInvalidationTest = CommandStreamerHelperTest;

HWTEST_F(TlbInvalidationTest, givenTlbInvalidationDescriptorSupportedWhenInvalidatingAllTlbsThenInvalidateAllDescriptorIsWrittenOnEveryDeviceWithDword0Last, HwMatcher::coreUsesAsidTlbInvalidationDescriptor) {
    constexpr uint32_t devicesCount = 2;
    EXPECT_CALL(stream, getStreamMode()).WillRepeatedly(Return(mode::tbx));
    EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(0);
    ::testing::InSequence sequence;
    for (uint32_t device = 0; device < devicesCount; device++) {
        const uint32_t mmioDevice = device * mmioDeviceOffset;
        EXPECT_CALL(stream, writeMMIO(mmioDevice + tlbInvalidationDescriptorDword1, 0u, 0xffffffff)).Times(1);
        EXPECT_CALL(stream, writeMMIO(mmioDevice + tlbInvalidationDescriptorDword0, invalidateAllMappingsDescriptorDword0, 0xffffffff)).Times(1);
        EXPECT_CALL(stream, readMMIO(mmioDevice + tlbInvalidationDescriptorDword0)).WillOnce(Return(0u));
    }

    EXPECT_TRUE(gpu->invalidateAllTLBs(stream, devicesCount));
}

HWTEST_F(TlbInvalidationTest, givenTlbInvalidationDescriptorSupportedAndAubFileStreamWhenAckIsReceivedThenAckPollIsRecordedOnEveryDevice, HwMatcher::coreUsesAsidTlbInvalidationDescriptor) {
    constexpr uint32_t devicesCount = 2;
    EXPECT_CALL(stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(stream, readMMIO(_)).WillRepeatedly(Return(0u));
    EXPECT_CALL(stream, getStreamMode()).WillRepeatedly(Return(mode::aubFile));
    for (uint32_t device = 0; device < devicesCount; device++) {
        EXPECT_CALL(stream, registerPoll(device * mmioDeviceOffset + tlbInvalidationDescriptorDword0, 0b1u, 0u, false, static_cast<uint32_t>(CmdServicesMemTraceRegisterPoll::TimeoutActionValues::Abort))).Times(1);
    }

    EXPECT_TRUE(gpu->invalidateAllTLBs(stream, devicesCount));
}

HWTEST_F(TlbInvalidationTest, givenTlbInvalidationDescriptorSupportedAndAckPendingWhenInvalidatingAllTlbsThenAckIsPolledUntilValidBitIsCleared, HwMatcher::coreUsesAsidTlbInvalidationDescriptor) {
    EXPECT_CALL(stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(stream, readMMIO(tlbInvalidationDescriptorDword0)).WillOnce(Return(1u)).WillOnce(Return(1u)).WillOnce(Return(0u));

    gpu->invalidateAllTLBs(stream, 1);
}

HWTEST_F(TlbInvalidationTest, givenTlbInvalidationDescriptorSupportedAndAckNeverReceivedWhenInvalidatingAllTlbsThenPollingStopsAfterMaxRetries, HwMatcher::coreUsesAsidTlbInvalidationDescriptor) {
    EXPECT_CALL(stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(stream, readMMIO(tlbInvalidationDescriptorDword0)).Times(maxAckStatusReads).WillRepeatedly(Return(1u));
    EXPECT_CALL(stream, getStreamMode()).WillRepeatedly(Return(mode::aubFile));
    EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(0);

    EXPECT_FALSE(gpu->invalidateAllTLBs(stream, 1));
}

HWTEST_F(TlbInvalidationTest, givenTlbInvalidationDescriptorNotSupportedWhenInvalidatingAllTlbsThenNoMmioIsAccessed, HwMatcher::tlbInvalidationDescriptorNotSupported) {
    EXPECT_CALL(stream, writeMMIO(_, _, _)).Times(0);
    EXPECT_CALL(stream, readMMIO(_)).Times(0);
    EXPECT_CALL(stream, registerPoll(_, _, _, _, _)).Times(0);

    EXPECT_TRUE(gpu->invalidateAllTLBs(stream, 1));
}

TEST(AubManagerTlbInvalidation, givenTlbInvalidationDescriptorSupportedWhenFreeingMemoryThenAllTlbsAreInvalidatedOnceOnEachFreeOnlyAfterAnyContextWasSubmitted) {
    TEST_REQUIRES(HwMatcher::coreUsesAsidTlbInvalidationDescriptor(gpu.get()));

    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.initialize();
    auto stream = aubManager.getMockAubFileStream();
    ASSERT_NE(nullptr, stream);

    EXPECT_CALL(*stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(*stream, freeMemory(_, _, _)).Times(::testing::AnyNumber());

    std::vector<HardwareContext *> contexts;
    for (uint32_t contextId : {3u, 5u}) {
        CreateHardwareContext2Params params = {contextId, hardwareContextId::invalidContextId};
        auto context = aubManager.createHardwareContext2(params, defaultDevice, defaultEngine, 0);
        context->initialize();
        contexts.push_back(context);
    }

    EXPECT_CALL(*stream, writeMMIO(tlbInvalidationDescriptorDword0, _, _)).Times(0);
    aubManager.freeMemory(0x1000000, 0x10000);
    ::testing::Mock::VerifyAndClearExpectations(stream);

    EXPECT_CALL(*stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(*stream, freeMemory(_, _, _)).Times(::testing::AnyNumber());
    contexts[0]->submitBatchBuffer(0x100, false);
    contexts[1]->submitBatchBuffer(0x100, false);

    EXPECT_CALL(*stream, writeMMIO(tlbInvalidationDescriptorDword1, 0u, 0xffffffff)).Times(2);
    EXPECT_CALL(*stream, writeMMIO(tlbInvalidationDescriptorDword0, invalidateAllMappingsDescriptorDword0, 0xffffffff)).Times(2);
    aubManager.freeMemory(0x1000000, 0x10000);
    aubManager.freeMemory(0x1010000, 0x10000);
    ::testing::Mock::VerifyAndClearExpectations(stream);

    for (auto context : contexts) {
        EXPECT_TRUE(aubManager.releaseHardwareContext(context));
    }
}

TEST(AubManagerTlbInvalidation, givenTlbInvalidationDescriptorNotSupportedAndSubmittedContextWhenFreeingMemoryThenInvalidateAllDescriptorIsNotWritten) {
    TEST_REQUIRES(HwMatcher::tlbInvalidationDescriptorNotSupported(gpu.get()));

    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.initialize();
    auto stream = aubManager.getMockAubFileStream();
    ASSERT_NE(nullptr, stream);

    EXPECT_CALL(*stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(*stream, freeMemory(_, _, _)).Times(::testing::AnyNumber());

    CreateHardwareContext2Params params = {3u, hardwareContextId::invalidContextId};
    auto context = aubManager.createHardwareContext2(params, defaultDevice, defaultEngine, 0);
    context->initialize();
    context->submitBatchBuffer(0x100, false);

    EXPECT_CALL(*stream, writeMMIO(tlbInvalidationDescriptorDword0, invalidateAllMappingsDescriptorDword0, _)).Times(0);
    aubManager.freeMemory(0x1000000, 0x10000);
    ::testing::Mock::VerifyAndClearExpectations(stream);

    EXPECT_TRUE(aubManager.releaseHardwareContext(context));
}

TEST(AubManagerTlbInvalidation, givenThrowOnErrorAndTlbInvalidationAckNeverReceivedWhenFreeingMemoryAfterSubmissionThenErrorIsThrown) {
    TEST_REQUIRES(HwMatcher::coreUsesAsidTlbInvalidationDescriptor(gpu.get()));

    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile, {}, true);
    aubManager.createMockAubFileStream = true;
    aubManager.initialize();
    auto stream = aubManager.getMockAubFileStream();
    ASSERT_NE(nullptr, stream);

    EXPECT_CALL(*stream, writeMMIO(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(*stream, freeMemory(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(*stream, readMMIO(_)).WillRepeatedly(Return(0u));
    EXPECT_CALL(*stream, readMMIO(tlbInvalidationDescriptorDword0)).WillRepeatedly(Return(1u));

    CreateHardwareContext2Params params = {3u, hardwareContextId::invalidContextId};
    auto context = aubManager.createHardwareContext2(params, defaultDevice, defaultEngine, 0);
    context->initialize();
    context->submitBatchBuffer(0x100, false);

    EXPECT_THROW(aubManager.freeMemory(0x1000000, 0x10000), std::runtime_error);
    ::testing::Mock::VerifyAndClearExpectations(stream);

    EXPECT_TRUE(aubManager.releaseHardwareContext(context));
}
