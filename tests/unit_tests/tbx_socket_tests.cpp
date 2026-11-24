/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/tbx_sockets_imp.h"
#include "mock_tbx_socket.h"
#include "gmock/gmock.h"

#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

extern "C" {
void socketStubResetSetsockopt(int returnValue);
int socketStubGetSetsockoptCallCount();
int socketStubGetSetsockoptLastLevel();
int socketStubGetSetsockoptLastOptionName();
int socketStubGetSetsockoptLastOptionValue();
}

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

TEST(TbxSocketsTest, givenHostNameThatCannotBeResolvedWhenInitIsCalledThenReturnFalse) {
    std::stringstream err;
    TbxSocketsImp tbxSocket(err);

    EXPECT_FALSE(tbxSocket.init("unresolvable.host", 4321, false));
}

TEST(TbxSocketsTest, givenNumericHostWhenInitIsCalledThenReturnTrue) {
    std::stringstream err;
    TbxSocketsImp tbxSocket(err);

    EXPECT_TRUE(tbxSocket.init("127.0.0.1", 4321, false));
}

TEST(TbxSocketsTest, givenSocketCreatedWhenInitIsCalledThenNagleAlgorithmIsDisabledOnTheSocket) {
    socketStubResetSetsockopt(0);
    std::stringstream err;
    TbxSocketsImp tbxSocket(err);

    EXPECT_TRUE(tbxSocket.init("127.0.0.1", 4321, false));
    EXPECT_EQ(1, socketStubGetSetsockoptCallCount());
    EXPECT_EQ(IPPROTO_TCP, socketStubGetSetsockoptLastLevel());
    EXPECT_EQ(TCP_NODELAY, socketStubGetSetsockoptLastOptionName());
    EXPECT_EQ(1, socketStubGetSetsockoptLastOptionValue());
    EXPECT_TRUE(err.str().empty());
}

TEST(TbxSocketsTest, givenDisablingNagleAlgorithmFailsWhenInitIsCalledThenWarningIsLoggedAndConnectionSucceeds) {
    socketStubResetSetsockopt(-1);
    std::stringstream err;
    TbxSocketsImp tbxSocket(err);

    EXPECT_TRUE(tbxSocket.init("127.0.0.1", 4321, false));
    EXPECT_EQ(1, socketStubGetSetsockoptCallCount());
    EXPECT_NE(std::string::npos, err.str().find("TCP_NODELAY"));
    socketStubResetSetsockopt(0);
}
