/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/page_table_walker.h"
#include "aub_services.h"
#include "aubstream/aubstream.h"
#include "aubstream/allocation_params.h"

namespace aub_stream {

struct GpuDescriptor;
using MMIOPair = std::pair<uint32_t, uint32_t>;
using MMIOList = std::vector<MMIOPair>;

using SteppingValues = CmdServicesMemTraceVersion::SteppingValues;
using AddressSpaceValues = CmdServicesMemTraceMemoryWrite::AddressSpaceValues;
using DataTypeHintValues = CmdServicesMemTraceMemoryWrite::DataTypeHintValues;

enum class CoreFamily : uint32_t;
struct AubStream {

    enum PageTableType {
        PAGE_TABLE_GGTT = 0,
        PAGE_TABLE_PPGTT = 1,
    };

    void expectMemory(GGTT *ggtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation);
    void expectMemory(PageTable *ppgtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation);

    void readMemory(PageTable *ppgtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize);
    void readMemory(GGTT *gtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize);

    std::vector<PageInfo> writeMemory(GGTT *ggtt, uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize = 4096);
    std::vector<PageInfo> writeMemory(PageTable *ppgtt, const AllocationParams &allocationParams);

    virtual bool mapGpuVa(PageTable *ppgtt, AllocationParams allocationParams, uint64_t physicalAddress);

    virtual void freeMemory(PageTable *ppgtt, uint64_t gfxAddress, size_t size);

    void cloneMemory(PageTable *ppgtt, const std::vector<PageInfo> &entries, const AllocationParams &allocationParams);

    void writeMemoryAndClonePageTables(PageTable *ppgtt, PageTable *ppgttForCloning[], uint32_t ppggtForCloningCount,
                                       uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize = 4096);

    void writePhysicalMemoryPages(const void *memory, size_t size, const std::vector<PageInfo> entries, int hint);

    virtual void addComment(const char *message) = 0;
    virtual bool init(int steppingValue, const GpuDescriptor &gpu) = 0;
    virtual void registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) = 0;
    virtual void writeMMIO(uint32_t offset, uint32_t value) = 0;
    virtual uint32_t readMMIO(uint32_t offset) = 0;
    virtual void declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) = 0;
    virtual void dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) = 0;
    virtual void dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) = 0;

    virtual uint32_t getStreamMode() const = 0;

    virtual ~AubStream() = default;
    AubStream() = default;
    AubStream(const AubStream &) = delete;
    AubStream &operator=(const AubStream &) = delete;

  protected:
    void writePages(const PageTableWalker &pageWalker, const void *memory, size_t size, int hint, bool pageTablesInLocalMemory,
                    uint32_t numAddressBits);
    void writePageWalkEntries(const PageTableWalker &pageWalker, bool pageTablesInLocalMemory, uint32_t numAddressBits);
    void writePpgttLevel2(const std::vector<PageEntryInfo> &pageWalkEntry, bool pageTablesInLocalMemory, uint32_t numAddressBits);
    void writePpgttLevel1(const std::vector<PageEntryInfo> &pageWalkEntry, bool pageTablesInLocalMemory, uint32_t numAddressBits);
    virtual void writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) = 0;
    virtual void expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) = 0;
    virtual void reserveContiguousPages(const std::vector<uint64_t> &entries) = 0;
    virtual void readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) = 0;
    virtual void writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) = 0;
    virtual void writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) = 0;
    virtual void writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) = 0;
    bool dumpBinSupported = false;
    bool dumpSurfaceSupported = false;
};

inline bool IsAnyTbxShmMode(uint32_t m) {
    return m == aub_stream::mode::tbxShm || m == aub_stream::mode::tbxShm3 || m == aub_stream::mode::tbxShm4;
}

inline bool IsAnyTbxMode(uint32_t m) {
    return m == aub_stream::mode::tbx || m == aub_stream::mode::aubFileAndTbx || IsAnyTbxShmMode(m);
}

} // namespace aub_stream
