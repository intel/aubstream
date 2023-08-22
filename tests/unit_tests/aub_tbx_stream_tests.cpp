/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_tbx_stream.h"
#include "aub_services.h"
#include "aubstream/aubstream.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"

#include "gtest/gtest.h"
#include <memory>

using namespace aub_stream;
using ::testing::_;

struct MockAubTbxStream : public AubTbxStream {
    using AubTbxStream::AubTbxStream;

    using AubTbxStream::expectMemoryTable;
    using AubTbxStream::readDiscontiguousPages;
    using AubTbxStream::reserveContiguousPages;
    using AubTbxStream::writeContiguousPages;
    using AubTbxStream::writeDiscontiguousPages;
};

TEST(AubTbxStream, RedirectMethodsToAubFileAndTbxStreams) {
    auto fileStream = std::make_unique<MockAubFileStream>();
    auto tbxStream = std::make_unique<MockTbxStream>();

    auto aubTbxStream = std::make_unique<MockAubTbxStream>(*fileStream.get(), *tbxStream.get());

    EXPECT_CALL(*fileStream, init(_, _)).Times(1);
    EXPECT_CALL(*tbxStream, init(_, _)).Times(1);

    EXPECT_CALL(*fileStream, addComment(_)).Times(1);
    EXPECT_CALL(*tbxStream, addComment(_)).Times(1);

    EXPECT_CALL(*fileStream, declareContextForDumping(_, _)).Times(1);
    EXPECT_CALL(*tbxStream, declareContextForDumping(_, _)).Times(1);

    EXPECT_CALL(*fileStream, dumpBufferBIN(_, _, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, dumpBufferBIN(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, dumpSurface(_, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, dumpSurface(_, _, _)).Times(1);

    EXPECT_CALL(*fileStream, registerPoll(_, _, _, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, registerPoll(_, _, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeMMIO(_, _)).Times(1);
    EXPECT_CALL(*tbxStream, writeMMIO(_, _)).Times(1);

    EXPECT_CALL(*tbxStream, readMMIO(_)).Times(1);

    EXPECT_CALL(*fileStream, expectMemoryTable(_, _, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, expectMemoryTable(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, reserveContiguousPages(_)).Times(1);
    EXPECT_CALL(*tbxStream, reserveContiguousPages(_)).Times(1);

    EXPECT_CALL(*fileStream, readDiscontiguousPages(_, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, readDiscontiguousPages(_, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeContiguousPages(_, _, _, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, writeContiguousPages(_, _, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeDiscontiguousPages(_, _, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, writeDiscontiguousPages(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeDiscontiguousPages(_, _, _)).Times(1);
    EXPECT_CALL(*tbxStream, writeDiscontiguousPages(_, _, _)).Times(1);

    aubTbxStream->init(1, *gpu);
    aubTbxStream->addComment("test");
    aubTbxStream->declareContextForDumping(0, nullptr);

    aubTbxStream->dumpBufferBIN(AubStream::PageTableType::PAGE_TABLE_PPGTT, 0x100000, 0x1000, 0);
    aubTbxStream->dumpSurface(AubStream::PageTableType::PAGE_TABLE_PPGTT, {0x100000, 0x1000, 1, 0x1000, 0x1ff, 4, 0, false, 1}, 0);

    aubTbxStream->registerPoll(10, 10, 10, false, 10);
    aubTbxStream->writeMMIO(20, 20);
    aubTbxStream->readMMIO(24);

    std::vector<PageInfo> writeInfoTable;
    aubTbxStream->expectMemoryTable(nullptr, 0, writeInfoTable, CmdServicesMemTraceMemoryCompare::CompareOperationValues::CompareEqual);

    std::vector<uint64_t> entries;
    aubTbxStream->reserveContiguousPages(entries);

    aubTbxStream->readDiscontiguousPages(nullptr, 0, writeInfoTable);

    aubTbxStream->writeContiguousPages(nullptr, 0, 0x10000, 0, 0);
    aubTbxStream->writeDiscontiguousPages(nullptr, 0, writeInfoTable, 0);

    std::vector<PageEntryInfo> entryInfoTable;
    aubTbxStream->writeDiscontiguousPages(entryInfoTable, 0, 0);
}

TEST(AubTbxStream, RedirectMethodsToTbxStreamOnlyWhenAubFileStreamIsPaused) {
    auto fileStream = std::make_unique<MockAubFileStream>();
    auto tbxStream = std::make_unique<MockTbxStream>();

    auto aubTbxStream = std::make_unique<MockAubTbxStream>(*fileStream.get(), *tbxStream.get());
    aubTbxStream->pauseAubFileStream(true);

    EXPECT_CALL(*fileStream, init(_, _)).Times(1);
    EXPECT_CALL(*tbxStream, init(_, _)).Times(1);

    EXPECT_CALL(*fileStream, addComment(_)).Times(0);
    EXPECT_CALL(*tbxStream, addComment(_)).Times(1);

    EXPECT_CALL(*fileStream, declareContextForDumping(_, _)).Times(0);
    EXPECT_CALL(*tbxStream, declareContextForDumping(_, _)).Times(1);

    EXPECT_CALL(*fileStream, dumpBufferBIN(_, _, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, dumpBufferBIN(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, dumpSurface(_, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, dumpSurface(_, _, _)).Times(1);

    EXPECT_CALL(*fileStream, registerPoll(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, registerPoll(_, _, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeMMIO(_, _)).Times(0);
    EXPECT_CALL(*tbxStream, writeMMIO(_, _)).Times(1);

    EXPECT_CALL(*tbxStream, readMMIO(_)).Times(1);

    EXPECT_CALL(*fileStream, expectMemoryTable(_, _, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, expectMemoryTable(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, reserveContiguousPages(_)).Times(0);
    EXPECT_CALL(*tbxStream, reserveContiguousPages(_)).Times(1);

    EXPECT_CALL(*fileStream, readDiscontiguousPages(_, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, readDiscontiguousPages(_, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeContiguousPages(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, writeContiguousPages(_, _, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeDiscontiguousPages(_, _, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, writeDiscontiguousPages(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeDiscontiguousPages(_, _, _)).Times(0);
    EXPECT_CALL(*tbxStream, writeDiscontiguousPages(_, _, _)).Times(1);

    aubTbxStream->init(1, *gpu);
    aubTbxStream->addComment("test");
    aubTbxStream->declareContextForDumping(0, nullptr);

    aubTbxStream->dumpBufferBIN(AubStream::PageTableType::PAGE_TABLE_PPGTT, 0x100000, 0x1000, 0);
    aubTbxStream->dumpSurface(AubStream::PageTableType::PAGE_TABLE_PPGTT, {0x100000, 0x1000, 1, 0x1000, 0x1ff, 4, 0, false, 1}, 0);

    aubTbxStream->registerPoll(10, 10, 10, false, 10);
    aubTbxStream->writeMMIO(20, 20);
    aubTbxStream->readMMIO(24);

    std::vector<PageInfo> writeInfoTable;
    aubTbxStream->expectMemoryTable(nullptr, 0, writeInfoTable, CmdServicesMemTraceMemoryCompare::CompareOperationValues::CompareEqual);

    std::vector<uint64_t> entries;
    aubTbxStream->reserveContiguousPages(entries);

    aubTbxStream->readDiscontiguousPages(nullptr, 0, writeInfoTable);

    aubTbxStream->writeContiguousPages(nullptr, 0, 0x10000, 0, 0);
    aubTbxStream->writeDiscontiguousPages(nullptr, 0, writeInfoTable, 0);

    std::vector<PageEntryInfo> entryInfoTable;
    aubTbxStream->writeDiscontiguousPages(entryInfoTable, 0, 0);
}

using AubShmStreamTest = ::testing::Test;
TEST(AubShmStreamTest, writeContiguousPagesInSHMModeWithCorrectValuesThenTranslateCallIsExpected) {
    MockTbxShmStream stream(mode::tbxShm);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal, &inVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AC000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });
    stream.baseWriteContiguousPages(&inVal, sizeof(inVal), 0x1AC000, aub_stream::AddressSpaceValues::TraceNonlocal, 0);
    EXPECT_EQ(inVal, outVal);
}

TEST(AubShmStreamTest, writeContiguousPagesInSHM3ModeWithCorrectValuesThenTranslateCallIsExpected) {
    MockTbxShmStream stream(mode::tbxShm3);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal, &inVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AC000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });
    stream.baseWriteContiguousPages(&inVal, sizeof(inVal), 0x1AC000, aub_stream::AddressSpaceValues::TraceNonlocal, 0);
    EXPECT_EQ(inVal, outVal);
}

TEST(AubShmStreamTest, writeContiguousPagesInSHM4ModeWithCorrectValuesThenTranslateCallIsExpected) {
    MockTbxShmStream stream(mode::tbxShm4);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal, &inVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AB000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });
    stream.baseWriteContiguousPages(&inVal, sizeof(inVal), 0x1AB000, aub_stream::AddressSpaceValues::TraceNonlocal, 0);
    EXPECT_EQ(inVal, outVal);
}