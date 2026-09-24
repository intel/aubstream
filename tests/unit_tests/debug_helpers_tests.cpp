/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/debug_helpers.h"
#include "debug_assert_fixture.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <string>

using namespace aub_stream;

namespace {

struct DebugAssertTest : public DebugAssertFixture, public ::testing::Test {
    void SetUp() override { DebugAssertFixture::SetUp(); }
    void TearDown() override { DebugAssertFixture::TearDown(); }
};

} // namespace

TEST(DebugAssertSetting, givenDefaultSettingsWhenCheckingEnableDebugAssertsThenItIsEnabled) {
    Settings settings;
    EXPECT_TRUE(settings.EnableDebugAsserts.get());
}

TEST_F(DebugAssertTest, givenDebugAssertsEnabledAndBreakHandledByDebuggerWhenHandlingDebugAssertFailureThenFailureIsReportedAndProcessIsNotAborted) {
    globalSettings->EnableDebugAsserts.set(true);
    mock_os_calls::breakHandledByDebugger = true;

    ::testing::internal::CaptureStderr();
    handleDebugAssertFailure("pageSize > 0", "page_table_walker.cpp", 42);
    std::string output = ::testing::internal::GetCapturedStderr();

    EXPECT_THAT(output, ::testing::HasSubstr("Assertion failed: pageSize > 0, file page_table_walker.cpp, line 42\n"));
    EXPECT_EQ(1u, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(0u, mock_os_calls::abortProcessCalled);
}

TEST_F(DebugAssertTest, givenDebugAssertsEnabledAndBreakNotHandledByDebuggerWhenHandlingDebugAssertFailureThenFailureIsReportedAndProcessIsAborted) {
    globalSettings->EnableDebugAsserts.set(true);
    mock_os_calls::breakHandledByDebugger = false;

    ::testing::internal::CaptureStderr();
    handleDebugAssertFailure("pageSize > 0", "page_table_walker.cpp", 42);
    std::string output = ::testing::internal::GetCapturedStderr();

    EXPECT_THAT(output, ::testing::HasSubstr("Assertion failed: pageSize > 0, file page_table_walker.cpp, line 42\n"));
    EXPECT_EQ(1u, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(1u, mock_os_calls::abortProcessCalled);
}

TEST_F(DebugAssertTest, givenDebugAssertsDisabledWhenHandlingDebugAssertFailureThenFailureIsSkipped) {
    globalSettings->EnableDebugAsserts.set(false);

    ::testing::internal::CaptureStderr();
    handleDebugAssertFailure("expression", "file.cpp", 1);
    std::string output = ::testing::internal::GetCapturedStderr();

    EXPECT_TRUE(output.empty());
    EXPECT_EQ(0u, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(0u, mock_os_calls::abortProcessCalled);
}

TEST_F(DebugAssertTest, givenTrueExpressionWhenDebugAssertIsCheckedThenFailureIsNotHandled) {
    globalSettings->EnableDebugAsserts.set(true);
    const bool conditionMet = true;

    AUBSTREAM_DEBUG_ASSERT(conditionMet);

    EXPECT_EQ(0u, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(0u, mock_os_calls::abortProcessCalled);
}

TEST_F(DebugAssertTest, givenFalseExpressionAndBreakHandledByDebuggerWhenDebugAssertIsCheckedThenBreakIsTriggeredOnlyInDebugBuild) {
    globalSettings->EnableDebugAsserts.set(true);
    mock_os_calls::breakHandledByDebugger = true;
    const bool conditionMet = false;

    ::testing::internal::CaptureStderr();
    AUBSTREAM_DEBUG_ASSERT(conditionMet);
    ::testing::internal::GetCapturedStderr();

    EXPECT_EQ(expectedCallsWhenDebugAssertsCompiledIn, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(0u, mock_os_calls::abortProcessCalled);
}

TEST_F(DebugAssertTest, givenFalseExpressionAndBreakNotHandledByDebuggerWhenDebugAssertIsCheckedThenProcessIsAbortedOnlyInDebugBuild) {
    globalSettings->EnableDebugAsserts.set(true);
    mock_os_calls::breakHandledByDebugger = false;
    const bool conditionMet = false;

    ::testing::internal::CaptureStderr();
    AUBSTREAM_DEBUG_ASSERT(conditionMet);
    std::string output = ::testing::internal::GetCapturedStderr();

    EXPECT_EQ(expectedCallsWhenDebugAssertsCompiledIn, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(expectedCallsWhenDebugAssertsCompiledIn, mock_os_calls::abortProcessCalled);
    EXPECT_EQ(debugAssertsCompiledIn, output.find("Assertion failed: conditionMet") != std::string::npos);
}

TEST_F(DebugAssertTest, givenFalseExpressionAndDebugAssertsDisabledWhenDebugAssertIsCheckedThenFailureIsSkipped) {
    globalSettings->EnableDebugAsserts.set(false);
    const bool conditionMet = false;

    AUBSTREAM_DEBUG_ASSERT(conditionMet);

    EXPECT_EQ(0u, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(0u, mock_os_calls::abortProcessCalled);
}

TEST_F(DebugAssertTest, givenExpressionWithSideEffectWhenDebugAssertIsCheckedThenExpressionIsEvaluatedOnlyInDebugBuild) {
    uint32_t evaluationCount = 0;

    AUBSTREAM_DEBUG_ASSERT(++evaluationCount > 0);

    EXPECT_EQ(expectedCallsWhenDebugAssertsCompiledIn, evaluationCount);
    EXPECT_EQ(0u, mock_os_calls::breakIntoDebuggerCalled);
    EXPECT_EQ(0u, mock_os_calls::abortProcessCalled);
}
