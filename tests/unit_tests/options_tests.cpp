/*
 * Copyright (C) 2022-2026 Intel Corporation
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
    GlobalVariableRestorer<MaskedMMIOList> tbxServerIpRestorer(&MMIOListInjected);
    MMIOList list = {MMIOPair(0xE48C, 0x20002), MMIOPair(0x1234, 0x40002)};
    injectMMIOList(list);

    EXPECT_EQ(2u, list.size());
}

TEST(OptionsTest, whenInjectMaskedMMIOListIsCalledThenMMIOsAreInjected) {
    GlobalVariableRestorer<MaskedMMIOList> tbxServerIpRestorer(&MMIOListInjected);
    MaskedMMIOList list = {MaskedMMIOWrite(0xE48C, 0x20002, 0xffffffff), MaskedMMIOWrite(0x1234, 0x40002, 0xffffffff)};
    injectMaskedMMIOList(list);

    EXPECT_EQ(2u, list.size());
}

TEST(OptionsTest, whenSetAubStreamCallerIsNEOThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(caller::neo);
    EXPECT_EQ(0u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "NEO");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(caller::rl);
    EXPECT_EQ(4u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RL");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLCThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(caller::rlc);
    EXPECT_EQ(2u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RLC");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLRThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(caller::rlr);
    EXPECT_EQ(1u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RLR");
}

TEST(OptionsTest, whenSetAubStreamCallerIsRLLThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(caller::rll);
    EXPECT_EQ(3u, aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "RLL");
}

TEST(OptionsTest, whenSetAubStreamCallerIsUnknownThenCorrectStringIsSet) {
    GlobalVariableRestorer<uint32_t> aubStreamCallerRestorer(&aubStreamCaller);

    char str[4] = {};

    setAubStreamCaller(std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), aubStreamCaller);

    getHeaderStr(aubStreamCaller, str);
    EXPECT_STREQ(str, "UNK");
}
