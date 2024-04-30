/*
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <fstream>
#include "aub_file_stream.h"
#include "tbx_shm_stream.h"

namespace aub_stream {

struct AubShmStream : public AubStream {
    AubShmStream(AubFileStream &aubFileStream, TbxShmStream &tbxShmStream) : aubFileStream(aubFileStream), tbxShmStream(tbxShmStream) {
    }

    virtual ~AubShmStream() = default;

    bool init(int steppingValue, const GpuDescriptor &gpu) override {
        auto result = aubFileStream.init(steppingValue, gpu);
        result &= tbxShmStream.init(steppingValue, gpu);
        return result;
    }

    void addComment(const char *message) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.addComment(message);
        }
        tbxShmStream.addComment(message);
    }

    void declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.declareContextForDumping(handleDumpContext, pageTable);
        }
        tbxShmStream.declareContextForDumping(handleDumpContext, pageTable);
    }

    void dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.dumpBufferBIN(gttType, gfxAddress, size, handleDumpContext);
        }
        tbxShmStream.dumpBufferBIN(gttType, gfxAddress, size, handleDumpContext);
    }

    void dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.dumpSurface(gttType, surfaceInfo, handleDumpContext);
        }
        tbxShmStream.dumpSurface(gttType, surfaceInfo, handleDumpContext);
    }

    void registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.registerPoll(registerOffset, mask, desiredValue, pollNotEqual, timeoutAction);
        }
        tbxShmStream.registerPoll(registerOffset, mask, desiredValue, pollNotEqual, timeoutAction);
    }

    void writeMMIO(uint32_t offset, uint32_t value) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeMMIO(offset, value);
        }
        tbxShmStream.writeMMIO(offset, value);
    }

    uint32_t readMMIO(uint32_t offset) override {
        return tbxShmStream.readMMIO(offset);
    }

    void writePCICFG(uint32_t offset, uint32_t value) override {
        if (!isAubFileStreamPaused) {
            tbxShmStream.writePCICFG(offset, value);
        }
        tbxShmStream.writePCICFG(offset, value);
    }
    uint32_t readPCICFG(uint32_t offset) override {
        return tbxShmStream.readPCICFG(offset);
    }

    uint32_t getStreamMode() const override { return aub_stream::mode::aubFileAndShm; };

    void pauseAubFileStream(bool onoff) { isAubFileStreamPaused = onoff; }
    void blockMemWritesViaTbxStream(bool onoff) { isTbxStreamBlockedForMemWrites = onoff; }

  protected:
    void writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeGttPages(ggtt, writeInfoTable);
        }
        tbxShmStream.writeGttPages(ggtt, writeInfoTable);
    }

    void expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.expectMemoryTable(memory, size, writeInfoTable, compareOperation);
        }
        tbxShmStream.expectMemoryTable(memory, size, writeInfoTable, compareOperation);
    }
    void reserveContiguousPages(const std::vector<uint64_t> &entries) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.reserveContiguousPages(entries);
        }
        tbxShmStream.reserveContiguousPages(entries);
    }
    void readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) override {
        tbxShmStream.readDiscontiguousPages(memory, size, writeInfoTable);
    }
    void writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeContiguousPages(memory, size, physAddress, addressSpace, hint);
        }
        if (!isTbxStreamBlockedForMemWrites) {
            tbxShmStream.writeContiguousPages(memory, size, physAddress, addressSpace, hint);
        }
    }
    void writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeDiscontiguousPages(memory, size, writeInfoTable, hint);
        }
        if (!isTbxStreamBlockedForMemWrites) {
            tbxShmStream.writeDiscontiguousPages(memory, size, writeInfoTable, hint);
        }
    }
    void writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) override {
        if (!isAubFileStreamPaused) {
            aubFileStream.writeDiscontiguousPages(writeInfoTable, addressSpace, hint);
        }
        if (!isTbxStreamBlockedForMemWrites) {
            tbxShmStream.writeDiscontiguousPages(writeInfoTable, addressSpace, hint);
        }
    }

    AubFileStream &aubFileStream;
    TbxShmStream &tbxShmStream;
    bool isAubFileStreamPaused = false;
    bool isTbxStreamBlockedForMemWrites = false;
};

} // namespace aub_stream
