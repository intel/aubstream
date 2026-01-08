/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <fstream>
#include "aub_file_stream.h"
#include "tbx_stream.h"

namespace aub_stream {

struct AubTbxStream : public AubStream {
    using AubStream::memoryPoll;

    AubTbxStream(AubFileStream &aubFileStream, TbxStream &tbxStream) : aubFileStream(aubFileStream), tbxStream(tbxStream) {
    }

    virtual ~AubTbxStream() = default;

    bool init(int steppingValue, const GpuDescriptor &gpu) override {
        auto result = aubFileStream.init(steppingValue, gpu);
        result &= tbxStream.init(steppingValue, gpu);
        return result;
    }

    void addComment(const char *message) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.addComment(message);
        }
        tbxStream.addComment(message);
    }

    void declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.declareContextForDumping(handleDumpContext, pageTable);
        }
        tbxStream.declareContextForDumping(handleDumpContext, pageTable);
    }

    void dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.dumpBufferBIN(gttType, gfxAddress, size, handleDumpContext);
        }
        tbxStream.dumpBufferBIN(gttType, gfxAddress, size, handleDumpContext);
    }

    void dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.dumpSurface(gttType, surfaceInfo, handleDumpContext);
        }
        tbxStream.dumpSurface(gttType, surfaceInfo, handleDumpContext);
    }

    void registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.registerPoll(registerOffset, mask, desiredValue, pollNotEqual, timeoutAction);
        }
        tbxStream.registerPoll(registerOffset, mask, desiredValue, pollNotEqual, timeoutAction);
    }

    void writeMMIO(uint32_t offset, uint32_t value) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeMMIO(offset, value);
        }
        tbxStream.writeMMIO(offset, value);
    }

    uint32_t readMMIO(uint32_t offset) override {
        return tbxStream.readMMIO(offset);
    }

    void writePCICFG(uint32_t offset, uint32_t value) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writePCICFG(offset, value);
        }
        tbxStream.writePCICFG(offset, value);
    }
    uint32_t readPCICFG(uint32_t offset) override {
        return tbxStream.readPCICFG(offset);
    }

    uint32_t getStreamMode() const override { return aub_stream::mode::aubFileAndTbx; };

    void pauseAubFileStream(bool onoff) { isAubFileStreamPaused = onoff; }

  protected:
    void writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeGttPages(ggtt, writeInfoTable);
        }
        tbxStream.writeGttPages(ggtt, writeInfoTable);
    }

    void expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.expectMemoryTable(memory, size, writeInfoTable, compareOperation);
        }
        tbxStream.expectMemoryTable(memory, size, writeInfoTable, compareOperation);
    }
    void reserveContiguousPages(const std::vector<uint64_t> &entries) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.reserveContiguousPages(entries);
        }
        tbxStream.reserveContiguousPages(entries);
    }
    void readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) override {
        tbxStream.readDiscontiguousPages(memory, size, writeInfoTable);
    }
    void writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeContiguousPages(memory, size, physAddress, addressSpace, hint);
        }
        tbxStream.writeContiguousPages(memory, size, physAddress, addressSpace, hint);
    }
    void writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeDiscontiguousPages(memory, size, writeInfoTable, hint);
        }
        tbxStream.writeDiscontiguousPages(memory, size, writeInfoTable, hint);
    }
    void writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeDiscontiguousPages(writeInfoTable, addressSpace, hint);
        }
        tbxStream.writeDiscontiguousPages(writeInfoTable, addressSpace, hint);
    }

    void memoryPoll(const std::vector<PageInfo> &entries, uint32_t value, uint32_t compareMode) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.memoryPoll(entries, value, compareMode);
        }
        tbxStream.memoryPoll(entries, value, compareMode);
    }

    AubFileStream &aubFileStream;
    TbxStream &tbxStream;
    bool isAubFileStreamPaused = false;
};

} // namespace aub_stream
