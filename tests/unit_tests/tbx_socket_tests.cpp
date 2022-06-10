/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/tbx_sockets_imp.h"
#include "gmock/gmock.h"

namespace aub_stream {

struct MockTbxSocketsImp : public TbxSocketsImp {
    using TbxSocketsImp::checkServerConfig;
    using TbxSocketsImp::frontdoorMode;

  public:
    MOCK_METHOD4(readMemoryExt, bool(uint64_t offset, void *data, size_t size, bool isLocalMem));
    MOCK_METHOD4(writeMemoryExt, bool(uint64_t offset, const void *data, size_t size, bool isLocalMem));
    MOCK_METHOD5(readPCICFG, bool(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t *value));
};

} // namespace aub_stream

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
