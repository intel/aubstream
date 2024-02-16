/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_stream.h"
#include "tbx_sockets.h"
#include "aubstream/shared_mem_info.h"
#include "alloc_tools.h"
#include <chrono>
#include <list>
#include <memory>
#include <functional>

namespace aub_stream {
struct AubTbxStream;
class TbxSockets;

struct TbxShmStream : public AubStream {
    typedef std::function<void(uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize)> TranslatePhysicalAddressToSystemMemoryFn;

    TbxShmStream(uint32_t m) : mode(m){};
    virtual ~TbxShmStream();

    void addComment(const char *message) override;
    void expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) override;
    void declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) override;
    void dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) override;
    void dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) override;
    bool init(int steppingValue, const GpuDescriptor &gpu) override;
    void registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t value, bool pollNotEqual, uint32_t timeoutAction) override;
    void reserveContiguousPages(const std::vector<uint64_t> &entries) override;
    void writeMMIO(uint32_t offset, uint32_t value) override;
    uint32_t readMMIO(uint32_t offset) override;
    void writePCICFG(uint32_t offset, uint32_t value) override;
    uint32_t readPCICFG(uint32_t offset) override;

    bool init(TranslatePhysicalAddressToSystemMemoryFn fn);
    void readMemory(PageTable *ppgtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize);
    void readMemory(GGTT *gtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize);

    std::vector<PageInfo> writeMemory(GGTT *ggtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize = 4096);
    std::vector<PageInfo> writeMemory(PageTable *ppgtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize = 4096);

    uint32_t getStreamMode() const override { return mode; };

    TbxSockets *socket = nullptr;
    friend AubTbxStream;
    SharedMemoryInfo *sharedMemoryInfo = nullptr;

    void writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) override;

    void enableThrowOnError(bool enabled);

  protected:
    void readContiguousPages(void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint);
    void readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) override;
    void writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) override;
    void writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) override;
    void writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) override;

    std::chrono::time_point<std::chrono::steady_clock> lastTimeCheck{};
    virtual void checkSocketAlive();

    TranslatePhysicalAddressToSystemMemoryFn translatePhysicalAddressToSystemMemory;
    uint32_t mode;
};

} // namespace aub_stream
