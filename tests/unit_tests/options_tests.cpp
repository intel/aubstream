/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "aub_mem_dump/options.h"
#include "headers/aubstream.h"

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
