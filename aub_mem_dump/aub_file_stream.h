/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <fstream>
#include "aub_stream.h"
#include "settings.h"
#include <string>

namespace aub_stream {
struct AubTbxStream;
struct AubShmStream;

struct FileHandleWrapper {
    bool write(const char *memory, size_t size) {
        fileHandle.write(memory, size);
        if (fileHandle.bad()) {
            const char *message = "write() to file failed\n";
            PRINT_LOG_ERROR(message, "");
            if (throwOnError) {
                throw std::runtime_error(message);
            }
            return false;
        }
        return true;
    }

    bool flush() {
        fileHandle.flush();
        if (fileHandle.bad()) {
            const char *message = "flush() to file failed\n";
            PRINT_LOG_ERROR(message, "");
            if (throwOnError) {
                throw std::runtime_error(message);
            }
            return false;
        }
        return true;
    }

    void clear() {
        fileHandle.clear();
    }

    bool close() {
        fileHandle.close();
        if (fileHandle.bad()) {
            const char *message = "close() file failed\n";
            PRINT_LOG_ERROR(message, "");
            if (throwOnError) {
                throw std::runtime_error(message);
            }
            return false;
        }
        return true;
    }
    bool open(const char *name, std::ios_base::openmode flags) {
        fileHandle.open(name, flags);
        if (fileHandle.bad()) {
            const char *message = "open() file failed\n";
            PRINT_LOG_ERROR(message, "");
            if (throwOnError) {
                throw std::runtime_error(message);
            }
            return false;
        }
        return true;
    }

    bool isOpen() {
        return fileHandle.is_open();
    }

    bool isBad() {
        return fileHandle.bad();
    }
    std::ofstream fileHandle;
    bool throwOnError = false;
};

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
    void writePCICFG(uint32_t offset, uint32_t value) override {}
    uint32_t readPCICFG(uint32_t offset) override { return 0; }

    uint32_t getStreamMode() const override { return aub_stream::mode::aubFile; };

    void open(const char *name);
    void close();
    bool isOpen();
    const std::string &getFileName();
    void write(const char *buffer, std::streamsize size);

    void enableThrowOnError(bool enabled) {
        fileHandle.throwOnError = enabled;
    }

    FileHandleWrapper fileHandle;
    std::string fileName;
    friend AubTbxStream;
    friend AubShmStream;

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
