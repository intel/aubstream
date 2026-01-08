/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_stream.h"
#include "tbx_sockets.h"

namespace aub_stream {
struct AubTbxStream;
class TbxSockets;

struct TbxStream : public AubStream {
    virtual ~TbxStream();

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

    uint32_t getStreamMode() const override { return aub_stream::mode::tbx; };

    TbxSockets *socket = nullptr;
    friend AubTbxStream;

  protected:
    void writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) override;

    void readContiguousPages(void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint);
    void readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) override;
    void writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) override;
    void writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) override;
    void writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) override;
    void memoryPoll(const std::vector<PageInfo> &entries, uint32_t value, uint32_t compareMode) override;
};

} // namespace aub_stream
