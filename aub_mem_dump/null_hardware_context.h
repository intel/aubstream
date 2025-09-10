/*
 * Copyright (C) 2024-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aubstream/hardware_context.h"

namespace aub_stream {

struct NullHardwareContext : public HardwareContext {
    void initialize() override {}
    void pollForCompletion() override {}
    void writeAndSubmitBatchBuffer(uint64_t gfxAddress, const void *batchBuffer, size_t size, uint32_t memoryBanks, size_t pageSize) override {}
    void writeMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) override {}
    void freeMemory(uint64_t gfxAddress, size_t size) override {}
    void expectMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation) override {}
    void readMemory(uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize) override {}
    void readGttMemory(uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize);
    void dumpBufferBIN(uint64_t gfxAddress, size_t size) override {}
    void dumpSurface(const SurfaceInfo &surfaceInfo) override {}
    void submitBatchBuffer(uint64_t gfxAddress, bool overrideRingHead) override {}
    void writeMemory2(AllocationParams allocationParams) override {}
    void writeMMIO(uint32_t offset, uint32_t value) override {}
    void pollForFenceCompletion() override {}
    uint32_t getCurrentFence(void) override { return 0; }
    uint32_t getExpectedFence(void) override { return 0; }
};

} // namespace aub_stream
