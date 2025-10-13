/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_tbx_stream.h"
#include "aub_mem_dump/aub_shm_stream.h"
#include "aub_services.h"
#include "aubstream/aubstream.h"
#include "mock_aub_stream.h"
#include "test_defaults.h"
#include "mock_tbx_socket.h"
#include "mock_aub_manager.h"

#include "gtest/gtest.h"
#include <memory>
#include "aubstream/hardware_context.h"

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

struct MockAubShmStream : public AubShmStream {
    using AubShmStream::AubShmStream;

    using AubShmStream::expectMemoryTable;
    using AubShmStream::readDiscontiguousPages;
    using AubShmStream::reserveContiguousPages;
    using AubShmStream::writeContiguousPages;
    using AubShmStream::writeDiscontiguousPages;
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
    aubTbxStream->expectMemoryTable(nullptr, 0, writeInfoTable, CompareOperationValues::CompareEqual);

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

    EXPECT_CALL(*fileStream, writePCICFG(_, _)).Times(0);
    EXPECT_CALL(*tbxStream, writePCICFG(_, _)).Times(1);

    EXPECT_CALL(*tbxStream, readPCICFG(_)).Times(1);

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

    aubTbxStream->writePCICFG(0x88, 30);
    aubTbxStream->readPCICFG(0x84);

    std::vector<PageInfo> writeInfoTable;
    aubTbxStream->expectMemoryTable(nullptr, 0, writeInfoTable, CompareOperationValues::CompareEqual);

    std::vector<uint64_t> entries;
    aubTbxStream->reserveContiguousPages(entries);

    aubTbxStream->readDiscontiguousPages(nullptr, 0, writeInfoTable);

    aubTbxStream->writeContiguousPages(nullptr, 0, 0x10000, 0, 0);
    aubTbxStream->writeDiscontiguousPages(nullptr, 0, writeInfoTable, 0);

    std::vector<PageEntryInfo> entryInfoTable;
    aubTbxStream->writeDiscontiguousPages(entryInfoTable, 0, 0);
}

TEST(AubTbxStream, RedirectMethodsToTbxStreamOnlyWhenTbxStreamIsPaused) {
    auto fileStream = std::make_unique<MockAubFileStream>();
    auto tbxShmStream = std::make_unique<MockTbxShmStream>(mode::aubFileAndShm);

    auto aubShmStream = std::make_unique<MockAubShmStream>(*fileStream.get(), *tbxShmStream.get());
    aubShmStream->blockMemWritesViaTbxStream(true);

    EXPECT_CALL(*fileStream, init(_, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, init(_, _)).Times(1);

    EXPECT_CALL(*fileStream, addComment(_)).Times(1);
    EXPECT_CALL(*tbxShmStream, addComment(_)).Times(1);

    EXPECT_CALL(*fileStream, declareContextForDumping(_, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, declareContextForDumping(_, _)).Times(1);

    EXPECT_CALL(*fileStream, dumpBufferBIN(_, _, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, dumpBufferBIN(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, dumpSurface(_, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, dumpSurface(_, _, _)).Times(1);

    EXPECT_CALL(*fileStream, registerPoll(_, _, _, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, registerPoll(_, _, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, writeMMIO(_, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, writeMMIO(_, _)).Times(1);

    EXPECT_CALL(*tbxShmStream, readMMIO(_)).Times(1);

    EXPECT_CALL(*tbxShmStream, readPCICFG(_)).Times(1);

    EXPECT_CALL(*fileStream, expectMemoryTable(_, _, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, expectMemoryTable(_, _, _, _)).Times(1);

    EXPECT_CALL(*fileStream, reserveContiguousPages(_)).Times(1);
    EXPECT_CALL(*tbxShmStream, reserveContiguousPages(_)).Times(1);

    EXPECT_CALL(*fileStream, writeContiguousPages(_, _, _, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, writeContiguousPages(_, _, _, _, _)).Times(0);

    EXPECT_CALL(*fileStream, writeDiscontiguousPages(_, _, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, writeDiscontiguousPages(_, _, _, _)).Times(0);

    EXPECT_CALL(*fileStream, writeDiscontiguousPages(_, _, _)).Times(1);
    EXPECT_CALL(*tbxShmStream, writeDiscontiguousPages(_, _, _)).Times(0);

    aubShmStream->init(1, *gpu);
    aubShmStream->addComment("test");
    aubShmStream->declareContextForDumping(0, nullptr);

    aubShmStream->dumpBufferBIN(AubStream::PageTableType::PAGE_TABLE_PPGTT, 0x100000, 0x1000, 0);
    aubShmStream->dumpSurface(AubStream::PageTableType::PAGE_TABLE_PPGTT, {0x100000, 0x1000, 1, 0x1000, 0x1ff, 4, 0, false, 1}, 0);

    aubShmStream->registerPoll(10, 10, 10, false, 10);
    aubShmStream->writeMMIO(20, 20);
    aubShmStream->readMMIO(24);

    aubShmStream->readPCICFG(0x84);

    std::vector<PageInfo> writeInfoTable;
    aubShmStream->expectMemoryTable(nullptr, 0, writeInfoTable, CompareOperationValues::CompareEqual);

    std::vector<uint64_t> entries;
    aubShmStream->reserveContiguousPages(entries);

    aubShmStream->writeContiguousPages(nullptr, 0, 0x10000, 0, 0);
    aubShmStream->writeDiscontiguousPages(nullptr, 0, writeInfoTable, 0);

    std::vector<PageEntryInfo> entryInfoTable;
    aubShmStream->writeDiscontiguousPages(entryInfoTable, 0, 0);
}

TEST(TbxStream, GivenMmioReadFailWhenPollingForCompletionThenFunctionReturnsEarly) {
    auto tbxStream = std::make_unique<MockTbxStream>();
    auto socket = new MockTbxSocketsImp();

    tbxStream->socket = socket;

    EXPECT_CALL(*socket, readMMIO(_, _)).Times(1).WillOnce(::testing::Return(false));

    EXPECT_CALL(*tbxStream, registerPoll(0x2234, 1, 1, false, _)).Times(1).WillOnce(::testing::Invoke([&](uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) { tbxStream->TbxStream::registerPoll(registerOffset, mask, desiredValue, pollNotEqual, timeoutAction); }));

    tbxStream->registerPoll(0x2234, 1, 1, false, CmdServicesMemTraceRegisterPoll::TimeoutActionValues::Abort);
}

TEST(TbxStream, GivenNoContextExecutedWhenPollingForCompletionThenFunctionReturnsEarly) {
    auto tbxStream = std::make_unique<MockTbxStream>();
    auto socket = new MockTbxSocketsImp();

    tbxStream->socket = socket;

    EXPECT_CALL(*socket, readMMIO(_, _)).Times(1).WillOnce(::testing::Invoke([&](uint32_t offset, uint32_t *data) { *data = 1<<12 | 1; return true; }));

    EXPECT_CALL(*tbxStream, registerPoll(0x2234, 1, 1, false, _)).Times(1).WillOnce(::testing::Invoke([&](uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) { tbxStream->TbxStream::registerPoll(registerOffset, mask, desiredValue, pollNotEqual, timeoutAction); }));

    tbxStream->registerPoll(0x2234, 1, 1, false, CmdServicesMemTraceRegisterPoll::TimeoutActionValues::Abort);
}

TEST(TbxStream, SocketProperClosingAtModeTbxWhenCloseSocketFunctionIsCall) {

    auto tbxStream = std::make_unique<MockTbxStream>();
    auto socket = new MockTbxSocketsImp();

    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbx);
    aubManager.initialize();

    tbxStream->socket = socket;

    EXPECT_CALL(*socket, close()).Times(1);

    aubManager.closeSocket();
}

TEST(TbxStream, SocketProperClosingAtModeAubTbxWhenCloseSocketFunctionIsCall) {

    auto tbxStream = std::make_unique<MockTbxStream>();
    auto socket = new MockTbxSocketsImp();

    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFileAndTbx);
    aubManager.initialize();

    tbxStream->socket = socket;

    EXPECT_CALL(*socket, close()).Times(1);

    aubManager.closeSocket();
}

using AubShmStreamTest = ::testing::Test;
TEST(AubShmStreamTest, writeContiguousPagesInSHMModeWithCorrectValuesThenTranslateCallIsExpected) {
    MockTbxShmStream stream(mode::tbxShm);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AC000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });
    EXPECT_CALL(stream, checkSocketAlive()).Times(1);
    stream.baseWriteContiguousPages(&inVal, sizeof(inVal), 0x1AC000, aub_stream::AddressSpaceValues::TraceNonlocal, 0);
    EXPECT_EQ(inVal, outVal);
}

TEST(AubShmStreamTest, writeContiguousPagesInSHM3ModeWithCorrectValuesThenTranslateCallIsExpected) {
    MockTbxShmStream stream(mode::tbxShm3);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AC000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });
    EXPECT_CALL(stream, checkSocketAlive()).Times(1);
    stream.baseWriteContiguousPages(&inVal, sizeof(inVal), 0x1AC000, aub_stream::AddressSpaceValues::TraceNonlocal, 0);
    EXPECT_EQ(inVal, outVal);
}

TEST(AubShmStreamTest, writeContiguousPagesInSHM4ModeWithCorrectValuesThenTranslateCallIsExpected) {
    MockTbxShmStream stream(mode::tbxShm4);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AB000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });
    EXPECT_CALL(stream, checkSocketAlive()).Times(1);
    stream.baseWriteContiguousPages(&inVal, sizeof(inVal), 0x1AB000, aub_stream::AddressSpaceValues::TraceNonlocal, 0);
    EXPECT_EQ(inVal, outVal);
}

TEST(AubShmStreamTest, SocketProperClosingAtModeTbxShmWhenCloseSocketFunctionIsCall) {
    MockTbxShmStream stream(mode::tbxShm);
    uint64_t inVal = 1977;
    uint64_t outVal = 0;
    stream.baseInit([&outVal](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
        EXPECT_EQ(physAddress, 0x1AB000);
        EXPECT_EQ(size, sizeof(inVal));
        p = &outVal;
        availableSize = size;
    });

    auto socket = new MockTbxSocketsImp();

    uint8_t sysMem[0x1000];
    uint8_t lMem[0x1000];
    SharedMemoryInfo sharedMemoryInfo = {sysMem, 0x1000, lMem, 0x1000};
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbxShm, sharedMemoryInfo);
    aubManager.initialize();

    stream.socket = socket;

    EXPECT_CALL(*socket, close()).Times(1);

    aubManager.closeSocket();
}
