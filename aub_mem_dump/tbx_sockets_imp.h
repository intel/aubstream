/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "tbx_sockets.h"
#include <iostream>
#include <mutex>

#ifdef _WIN32
#ifndef _WIN32_LEAN_AND_MEAN
#define _WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif
#else
typedef int SOCKET;
#endif

namespace aub_stream {
class TbxSocketsImp : public TbxSockets {
  public:
    explicit TbxSocketsImp(std::ostream &err = std::cerr);
    ~TbxSocketsImp() override = default;

    bool stillConnected() override;

    bool init(const std::string &hostNameOrIp, uint16_t port, bool frontdoor) override;
    void close() override;

    bool writeGTT(uint32_t gttOffset, uint64_t entry) override;

    bool readMemory(uint64_t offset, void *data, size_t size, bool isLocalMem) override;
    bool writeMemory(uint64_t offset, const void *data, size_t size, bool isLocalMem) override;

    bool readMemoryExt(uint64_t offset, void *data, size_t size, bool isLocalMem) override;
    bool writeMemoryExt(uint64_t offset, const void *data, size_t size, bool isLocalMem) override;

    bool readMMIO(uint32_t offset, uint32_t *data) override;
    bool writeMMIO(uint32_t offset, uint32_t data) override;

    bool readPCICFG(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t *value) override;
    bool writePCICFG(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t data) override;

    void enableThrowOnError(bool enabled) override;

  protected:
    std::ostream &cerrStream;
    SOCKET m_socket = 0;

    bool connectToServer(const std::string &hostNameOrIp, uint16_t port);
    bool checkServerConfig(bool frontdoor);
    bool sendWriteData(const void *buffer, size_t sizeInBytes);
    bool getResponseData(void *buffer, size_t sizeInBytes);

    inline uint32_t getNextTransID() { return transID++; }

    void logErrorInfo(const char *tag);

    uint32_t transID = 0;

    bool frontdoorMode = false;
    uint64_t lmembar = 0;

    bool inErrorState = false;
    std::mutex socket_mutex{};

    bool throwOnError = false;
};

} // namespace aub_stream
