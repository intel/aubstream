/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/tbx_sockets_imp.h"
#include "mock_tbx_socket.h"
#include "gmock/gmock.h"

using namespace aub_stream;
using ::testing::_;

TEST(TbxSocketsTest, givenTbxServerInFrontdoorModeWhenCheckServerConfigIsCalledThenReadPCICFGForLMEMBAR) {
    MockTbxSocketsImp tbxSocket;
    bool frontdoorMode = true;

    EXPECT_CALL(tbxSocket, readPCICFG(0, 2, 0, 0x18, _)).Times(1);
    EXPECT_CALL(tbxSocket, readPCICFG(0, 2, 0, 0x1C, _)).Times(1);

    tbxSocket.checkServerConfig(frontdoorMode);
    EXPECT_TRUE(tbxSocket.frontdoorMode);
}

TEST(TbxSocketsTest, givenTbxServerInFrontdoorModeWhenReadOrWriteMemoryIsCalledThenRedirectToReadOrWriteMemoryExtWithLMEMBAR) {
    MockTbxSocketsImp tbxSocket;
    tbxSocket.frontdoorMode = true;

    EXPECT_CALL(tbxSocket, readMemoryExt(_, _, _, true)).Times(1);
    EXPECT_CALL(tbxSocket, writeMemoryExt(_, _, _, true)).Times(1);

    tbxSocket.readMemory(0, nullptr, 0, true);
    tbxSocket.writeMemory(0, nullptr, 0, true);
}

TEST(TbxSocketsTest, givenTbxSocketInErrorStateWhenReadMMIOIsCalledThenReturnEarly) {
    MockTbxSocketsImp tbxSocket;
    tbxSocket.inErrorState = true;
    EXPECT_CALL(tbxSocket, readMMIO(_, _)).Times(1).WillOnce([&](uint32_t offset, uint32_t *data) {
        return tbxSocket.TbxSocketsImp::readMMIO(offset, data);
    });

    EXPECT_CALL(tbxSocket, sendWriteData(_, _)).Times(0);
    EXPECT_CALL(tbxSocket, getResponseData(_, _)).Times(0);

    EXPECT_FALSE(tbxSocket.readMMIO(0, nullptr));
}
