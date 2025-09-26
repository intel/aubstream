/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/tbx_sockets_imp.h"
#include "gmock/gmock.h"

namespace aub_stream {

struct MockTbxSocketsImp : public TbxSocketsImp {
    using TbxSocketsImp::checkServerConfig;
    using TbxSocketsImp::frontdoorMode;
    using TbxSocketsImp::inErrorState;

  public:
    MOCK_METHOD4(readMemoryExt, bool(uint64_t offset, void *data, size_t size, bool isLocalMem));
    MOCK_METHOD4(writeMemoryExt, bool(uint64_t offset, const void *data, size_t size, bool isLocalMem));
    MOCK_METHOD5(readPCICFG, bool(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t *value));
    MOCK_METHOD2(readMMIO, bool(uint32_t offset, uint32_t *data));
    MOCK_METHOD2(sendWriteData, bool(const void *buffer, size_t sizeInBytes));
    MOCK_METHOD2(getResponseData, bool(void *buffer, size_t sizeInBytes));
    MOCK_METHOD0(close, void(void));
};
} // namespace aub_stream
