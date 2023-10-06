/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <fstream>
#include "aub_stream.h"

namespace aub_stream {
struct AubTbxStream;

struct AubFileStream : public AubStream {
    virtual ~AubFileStream();

    bool init(int steppingValue, const GpuDescriptor &gpu) override;
    void addComment(const char *message) override;

    void declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) override;
    void dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) override;
    void dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) override;

    void registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) override;
    void writeMMIO(uint32_t offset, uint32_t value) override;
    uint32_t readMMIO(uint32_t offset) override {
        return 0;
    }

    uint32_t getStreamMode() const override { return aub_stream::mode::aubFile; };

    void open(const char *name);
    void close();
    bool isOpen();
    const std::string &getFileName();

    std::ofstream fileHandle;
    std::string fileName;
    friend AubTbxStream;

  protected:
    void writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) override;

    void expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) override;
    void reserveContiguousPages(const std::vector<uint64_t> &entries) override;
    void readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) override;
    void writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) override;
    void writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) override;
    void writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) override;
};

void getHeaderStr(uint32_t caller, char *header);

} // namespace aub_stream
