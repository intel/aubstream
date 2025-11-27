/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_file_stream.h"
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/tbx_stream.h"
#include "aub_mem_dump/tbx_shm_stream.h"
#include "aub_mem_dump/aub_shm_stream.h"
#include "gmock/gmock.h"
#include "tests/unit_tests/white_box.h"

namespace aub_stream {

template <>
struct WhiteBox<AubFileStream> : public AubFileStream {
    using AubStream::dumpBinSupported;
    using AubStream::dumpSurfaceSupported;
    WhiteBox() : AubFileStream() {}
};

struct MockAubStreamBase : public AubStream {
  public:
    MOCK_METHOD1(addComment, void(const char *message));
    MOCK_METHOD4(expectMemoryTable, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation));
    MOCK_METHOD2(init, bool(int steppingValue, const GpuDescriptor &gpu));
    MOCK_METHOD5(registerPoll, void(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction));
    MOCK_METHOD2(writeMMIO, void(uint32_t offset, uint32_t value));
    MOCK_METHOD1(readMMIO, uint32_t(uint32_t offset));
    MOCK_METHOD2(writePCICFG, void(uint32_t offset, uint32_t value));
    MOCK_METHOD1(readPCICFG, uint32_t(uint32_t offset));
    MOCK_METHOD5(writeContiguousPages, void(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint));
    MOCK_METHOD1(reserveContiguousPages, void(const std::vector<uint64_t> &entries));
    MOCK_METHOD3(readDiscontiguousPages, void(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable));
    MOCK_METHOD4(writeDiscontiguousPages, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint));
    MOCK_METHOD3(writeDiscontiguousPages, void(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint));
    MOCK_METHOD2(declareContextForDumping, void(uint32_t handleDumpContext, PageTable *pageTable));
    MOCK_METHOD4(dumpBufferBIN, void(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext));
    MOCK_METHOD3(dumpSurface, void(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext));
    MOCK_CONST_METHOD0(getStreamMode, uint32_t(void));
    MOCK_METHOD2(writeGttPages, void(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable));
    MOCK_METHOD5(phyToSys, void(uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize));
    MOCK_METHOD5(mapMemoryToPhysicalSpace, void(uint64_t physAddress, size_t size, size_t alignment, bool isLocalMemory, const void *p));
    MOCK_METHOD3(mapGpuVa, bool(PageTable *ppgtt, AllocationParams allocationParams, uint64_t physicalAddress));
    MOCK_METHOD6(readMemory, void(GGTT *ggtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize));
    MOCK_METHOD4(writePhysicalMemoryPages, void(const void *memory, size_t size, const std::vector<PageInfo> entries, int hint));
};

struct MockAubFileStream : public AubFileStream {
  public:
    MOCK_METHOD2(init, bool(int steppingValue, const GpuDescriptor &gpu));
    MOCK_METHOD1(addComment, void(const char *message));

    MOCK_METHOD2(declareContextForDumping, void(uint32_t handleDumpContext, PageTable *pageTable));
    MOCK_METHOD4(dumpBufferBIN, void(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext));
    MOCK_METHOD3(dumpSurface, void(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext));

    MOCK_METHOD5(registerPoll, void(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction));
    MOCK_METHOD1(readMMIO, uint32_t(uint32_t offset));
    MOCK_METHOD2(writeMMIO, void(uint32_t offset, uint32_t value));
    MOCK_METHOD2(writePCICFG, void(uint32_t offset, uint32_t value));
    MOCK_METHOD4(expectMemoryTable, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation));
    MOCK_METHOD1(reserveContiguousPages, void(const std::vector<uint64_t> &entries));
    MOCK_METHOD3(readDiscontiguousPages, void(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable));
    MOCK_METHOD5(writeContiguousPages, void(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint));
    MOCK_METHOD4(writeDiscontiguousPages, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint));
    MOCK_METHOD3(writeDiscontiguousPages, void(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint));
    MOCK_METHOD3(freeMemory, void(PageTable *ppgtt, uint64_t gfxAddress, size_t size));
    MOCK_METHOD2(writeGttPages, void(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable));
    MOCK_METHOD3(mapGpuVa, bool(PageTable *ppgtt, AllocationParams allocationParams, uint64_t physicalAddress));
    MOCK_METHOD6(readMemory, void(GGTT *ggtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize));
};

struct MockTbxStream : public TbxStream {
  public:
    MOCK_METHOD2(init, bool(int steppingValue, const GpuDescriptor &gpu));
    MOCK_METHOD1(addComment, void(const char *message));

    MOCK_METHOD2(declareContextForDumping, void(uint32_t handleDumpContext, PageTable *pageTable));
    MOCK_METHOD4(dumpBufferBIN, void(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext));
    MOCK_METHOD3(dumpSurface, void(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext));

    MOCK_METHOD5(registerPoll, void(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction));
    MOCK_METHOD2(writeMMIO, void(uint32_t offset, uint32_t value));
    MOCK_METHOD1(readMMIO, uint32_t(uint32_t offset));
    MOCK_METHOD2(writePCICFG, void(uint32_t offset, uint32_t value));
    MOCK_METHOD1(readPCICFG, uint32_t(uint32_t offset));
    MOCK_METHOD4(expectMemoryTable, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation));
    MOCK_METHOD1(reserveContiguousPages, void(const std::vector<uint64_t> &entries));
    MOCK_METHOD3(readDiscontiguousPages, void(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable));
    MOCK_METHOD5(writeContiguousPages, void(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint));
    MOCK_METHOD4(writeDiscontiguousPages, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint));
    MOCK_METHOD3(writeDiscontiguousPages, void(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint));
    MOCK_METHOD2(writeGttPages, void(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable));
    MOCK_METHOD3(mapGpuVa, bool(PageTable *ppgtt, AllocationParams allocationParams, uint64_t physicalAddress));
    MOCK_METHOD6(readMemory, void(GGTT *ggtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize));
};

struct MockReadMMIOTbxStream : public TbxStream {
  public:
    MOCK_METHOD1(readMMIO, uint32_t(uint32_t offset));
};

struct MockReadMMIOTbxShmStream : public TbxShmStream {
  public:
    MockReadMMIOTbxShmStream(uint32_t mode) : TbxShmStream(mode) {}
    MOCK_METHOD1(readMMIO, uint32_t(uint32_t offset));
};

struct MockTbxShmStream : public TbxShmStream {
  public:
    MockTbxShmStream(uint32_t mode) : TbxShmStream(mode) {}
    bool baseInit(TranslatePhysicalAddressToSystemMemoryFn fn) { return TbxShmStream::init(fn); }
    void baseWriteContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) { TbxShmStream::writeContiguousPages(memory, size, physAddress, addressSpace, hint); }
    MOCK_METHOD2(init, bool(int steppingValue, const GpuDescriptor &gpu));
    MOCK_METHOD1(addComment, void(const char *message));

    MOCK_METHOD2(declareContextForDumping, void(uint32_t handleDumpContext, PageTable *pageTable));
    MOCK_METHOD4(dumpBufferBIN, void(aub_stream::AubStream::PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext));
    MOCK_METHOD3(dumpSurface, void(aub_stream::AubStream::PageTableType gttType, const aub_stream::SurfaceInfo &surfaceInfo, uint32_t handleDumpContext));

    MOCK_METHOD5(registerPoll, void(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction));
    MOCK_METHOD2(writeMMIO, void(uint32_t offset, uint32_t value));
    MOCK_METHOD1(readMMIO, uint32_t(uint32_t offset));
    MOCK_METHOD2(writePCICFG, void(uint32_t offset, uint32_t value));
    MOCK_METHOD1(readPCICFG, uint32_t(uint32_t offset));

    MOCK_METHOD4(expectMemoryTable, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation));
    MOCK_METHOD1(reserveContiguousPages, void(const std::vector<uint64_t> &entries));
    MOCK_METHOD3(readDiscontiguousPages, void(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable));
    MOCK_METHOD5(writeContiguousPages, void(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint));
    MOCK_METHOD4(writeDiscontiguousPages, void(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint));
    MOCK_METHOD3(writeDiscontiguousPages, void(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint));
    MOCK_METHOD2(writeGttPages, void(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable));
    MOCK_METHOD3(mapGpuVa, bool(PageTable *ppgtt, AllocationParams allocationParams, uint64_t physicalAddress));
    MOCK_METHOD6(readMemory, void(GGTT *ggtt, uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize));
    MOCK_METHOD0(checkSocketAlive, void());
};

using MockAubStream = ::testing::NiceMock<MockAubStreamBase>;

struct MockAubStreamFixture {
    void SetUp() {
    }

    void TearDown() {
    }

    MockAubStream stream;
};

struct VerifyMmioAubStream : public MockAubStream {
    VerifyMmioAubStream(uint32_t min, uint32_t max) : mmioMin(min), mmioMax(max) {
    }

    void writeMMIO(uint32_t offset, uint32_t value) {
        assert(offset >= mmioMin);
        assert(offset <= mmioMax);
        EXPECT_GE(offset, mmioMin);
        EXPECT_LE(offset, mmioMax);
    }

    uint32_t mmioMin;
    uint32_t mmioMax;
};

} // namespace aub_stream
