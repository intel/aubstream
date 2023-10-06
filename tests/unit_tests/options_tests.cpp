/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/options.h"
#include "aubstream/aubstream.h"
#include "aub_mem_dump/aub_file_stream.h"

using namespace aub_stream;

template <typename T>
class GlobalVariableRestorer {
  public:
    GlobalVariableRestorer(T *ptr) : previous(ptr) {
        previousValue = *ptr;
    }

    ~GlobalVariableRestorer() {
        *previous = previousValue;
    }

  protected:
    T *previous;
    T previousValue;
};

TEST(OptionsTest, whenTbxServerIpIsSetToNewValueThenCorrectValueIsSet) {
    GlobalVariableRestorer<std::string> tbxServerIpRestorer(&tbxServerIp);

    std::string newIp("122.1.1.1");
    setTbxServerIp(newIp);
    EXPECT_STREQ(newIp.c_str(), tbxServerIp.c_str());
}

TEST(OptionsTest, whenTbxServerPortIsSetToNewValueThenCorrectValueIsSet) {
    GlobalVariableRestorer<uint16_t> tbxServerIpRestorer(&tbxServerPort);

    setTbxServerPort(55u);
    EXPECT_EQ(55u, tbxServerPort);
}

TEST(OptionsTest, whenTbxServerModeIsSetToNewValueThenCorrectValueIsSet) {
    GlobalVariableRestorer<bool> tbxServerIpRestorer(&tbxFrontdoorMode);

    setTbxFrontdoorMode(true);
    EXPECT_TRUE(tbxFrontdoorMode);
}

TEST(OptionsTest, whenInjectMMIOListIsCalledThenMMIOsAreInjected) {
    GlobalVariableRestorer<MMIOList> tbxServerIpRestorer(&MMIOListInjected);
    MMIOList list = {MMIOPair(0xE48C, 0x20002), MMIOPair(0x1234, 0x40002)};
    injectMMIOList(list);

    EXPECT_EQ(2u, list.size());
}

TEST(OptionsTest, whenSetAubStreamCallerIsNEOThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(16u);
    EXPECT_EQ(16u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "NEO");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(32u);
    EXPECT_EQ(32u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RL");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLCThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(34u);
    EXPECT_EQ(34u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RLC");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLRThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(33u);
    EXPECT_EQ(33u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RLR");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLLThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(35u);
    EXPECT_EQ(35u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RLL");
}

TEST(OptionsTest, whenSetAubStreamCallerIsUnknownThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(0u);
    EXPECT_EQ(0u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "UNK");
}
