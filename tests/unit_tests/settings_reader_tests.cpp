/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/settings.h"
#include "aub_mem_dump/settings_reader.h"
#include <string>
#include <unordered_map>

using namespace aub_stream;

namespace mock_os_calls {
extern std::unordered_map<std::string, std::string> environmentStrings;
}

TEST(EnvReader, givenEnvironmentWhenReadingSettingThenCorrectValuesReturned) {

    mock_os_calls::environmentStrings["first"] = "6";
    mock_os_calls::environmentStrings["second"] = "test";
    mock_os_calls::environmentStrings["third"] = "123456789";
    mock_os_calls::environmentStrings["fourth"] = "0";

    EnvironmentReader reader;

    int32_t value = reader.getSetting("first", 0);
    int64_t value64 = reader.getSetting("third", 0);
    bool logical = reader.getSetting("fourth", true);
    std::string test = reader.getSetting("second", std::string("abc"));

    EXPECT_EQ(6, value);
    EXPECT_EQ(123456789, value64);
    EXPECT_FALSE(logical);
    EXPECT_STREQ("test", test.c_str());

    mock_os_calls::environmentStrings.clear();

    value = reader.getSetting("first", 4);
    value64 = reader.getSetting("third", 5);
    logical = reader.getSetting("fourth", true);
    test = reader.getSetting("second", std::string("abc"));

    EXPECT_EQ(4, value);
    EXPECT_EQ(5, value64);
    EXPECT_TRUE(logical);
    EXPECT_STREQ("abc", test.c_str());
}

TEST(Settings, givenEnvironmentSettingPresentWhenGettingSettingThenCorrectValueReturned) {
    if (Settings::disabled()) {
        GTEST_SKIP();
    }
    mock_os_calls::environmentStrings["AUBSTREAM_PrintSettings"] = "1";
    std::stringstream outStream;
    Settings settings(&outStream);

    EXPECT_TRUE(settings.PrintSettings.get());

    EXPECT_STREQ("AUBSTREAM_PrintSettings = 1\n", outStream.str().c_str());

    mock_os_calls::environmentStrings.clear();
}

TEST(Settings, givenPrintSettingsEnvWhenSettingsCreatedThenNonDefaultSettingsArePrinted) {
    if (Settings::disabled()) {
        GTEST_SKIP();
    }
    mock_os_calls::environmentStrings["AUBSTREAM_PrintSettings"] = "1";

    ::testing::internal::CaptureStdout();
    Settings settings;

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("AUBSTREAM_PrintSettings = 1\n", output.c_str());

    mock_os_calls::environmentStrings.clear();
}