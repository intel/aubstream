/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <string>
#include <cstdint>

namespace aub_stream {

class TbxSockets {
  protected:
    TbxSockets() = default;

  public:
    virtual ~TbxSockets() = default;
    virtual bool init(const std::string &hostNameOrIp, uint16_t port, bool frontdoor) = 0;
    virtual void close() = 0;
    virtual bool stillConnected() = 0;

    virtual bool writeGTT(uint32_t offset, uint64_t entry) = 0;

    virtual bool readMemory(uint64_t addr, void *memory, size_t size, bool isLocalMemory) = 0;
    virtual bool writeMemory(uint64_t addr, const void *memory, size_t size, bool isLocalMemory) = 0;

    virtual bool readMemoryExt(uint64_t offset, void *data, size_t size, bool isLocalMem) = 0;
    virtual bool writeMemoryExt(uint64_t offset, const void *data, size_t size, bool isLocalMem) = 0;

    virtual bool readMMIO(uint32_t offset, uint32_t *value) = 0;
    virtual bool writeMMIO(uint32_t offset, uint32_t value) = 0;

    virtual bool readPCICFG(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t *value) = 0;

    virtual void enableThrowOnError(bool enabled) = 0;

    static TbxSockets *create();
};

} // namespace aub_stream
