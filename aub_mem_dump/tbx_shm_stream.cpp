/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/memory_bank_helper.h"
#include "aub_mem_dump/page_table_entry_bits.h"
#include "tbx_shm_stream.h"
#include "tbx_sockets.h"
#include "memcpy_s.h"
#include "options.h"
#include <cassert>
#include <sstream>
#include <iostream>
#include <cstring>

namespace aub_stream {

TbxShmStream::~TbxShmStream() {
    if (socket) {
        socket->close();
        delete socket;
    }
}

void TbxShmStream::addComment(const char *message) {
    log << message << std::endl;
}

void TbxShmStream::expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) {
}

void TbxShmStream::declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) {
}

void TbxShmStream::dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) {
}

void TbxShmStream::dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) {
}

void TbxShmStream::reserveContiguousPages(const std::vector<uint64_t> &entries) {
}

bool TbxShmStream::init(int stepping, const GpuDescriptor &gpu) {
    assert(0); // Shouldn't ever get here!
    return false;
}

bool TbxShmStream::init(TranslatePhysicalAddressToSystemMemoryFn fn) {
    socket = TbxSockets::create();
    assert(socket != nullptr);

    translatePhysicalAddressToSystemMemory = fn;

    return socket->init(tbxServerIp, tbxServerPort, tbxFrontdoorMode);
}

void TbxShmStream::readContiguousPages(void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
    void *p;
    size_t availableSize;
    translatePhysicalAddressToSystemMemory(physAddress, size, isLocalMemory, p, availableSize);
    // read shared mem
    memcpy_s(memory, availableSize, p, size);
    checkSocketAlive();
}

void TbxShmStream::readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) {
    for (auto &entry : writeInfoTable) {
        void *p;
        size_t availableSize;
        translatePhysicalAddressToSystemMemory(entry.physicalAddress, entry.size, entry.isLocalMemory, p, availableSize);
        // read shared mem
        memcpy_s(memory, availableSize, p, entry.size);

        memory = entry.size + (uint8_t *)memory;
    }
    checkSocketAlive();
}

void TbxShmStream::registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) {
    bool matches = false;
    do {
        uint32_t value = 0;
        socket->readMMIO(registerOffset, &value);

        matches = ((value & mask) == desiredValue);
    } while (matches == pollNotEqual);
}

void TbxShmStream::writeMMIO(uint32_t offset, uint32_t value) {
    log << "MMIO write: " << std::hex << std::showbase << offset
        << "   =: " << std::hex << std::showbase << value;

    socket->writeMMIO(offset, value);
}

uint32_t TbxShmStream::readPCICFG(uint32_t offset) {
    uint32_t value;
    socket->readPCICFG(0, 2, 0, offset, &value);

    log << "PCICFG read: " << std::hex << std::showbase << offset
        << "   =: " << std::hex << std::showbase << value;
    return value;
}

void TbxShmStream::writePCICFG(uint32_t offset, uint32_t value) {
    log << "PCICFG write: " << std::hex << std::showbase << offset
        << "   =: " << std::hex << std::showbase << value;

    socket->writePCICFG(0, 2, 0, offset, value);
}

uint32_t TbxShmStream::readMMIO(uint32_t offset) {
    uint32_t value;
    socket->readMMIO(offset, &value);

    log << "MMIO read: " << std::hex << std::showbase << offset
        << "   =: " << std::hex << std::showbase << value;
    return value;
}

void TbxShmStream::writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
    size_t destSize = 0;
    void *p = nullptr;
    translatePhysicalAddressToSystemMemory(physAddress, size, isLocalMemory, p, destSize);
    memcpy_s(p, destSize, memory, size);
    log << (isLocalMemory ? "Local" : "System") << " Mem: \t" << std::hex << physAddress << " \t" << std::hex << "0x00000000"
        << " \t" << std::hex << size << std::endl;
    checkSocketAlive();
}

void TbxShmStream::writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) {
    for (auto &entry : writeInfoTable) {
        size_t destSize = 0;
        void *p = nullptr;

        translatePhysicalAddressToSystemMemory(entry.physicalAddress, entry.size, entry.isLocalMemory, p, destSize);
        memcpy_s(p, destSize, memory, entry.size);

        log << (entry.isLocalMemory ? "Local" : "System") << " Mem: \t" << std::hex << entry.physicalAddress << " \t" << std::hex << "0x00000000"
            << " \t" << std::hex << entry.size << std::endl;

        memory = entry.size + (uint8_t *)memory;
    }
    checkSocketAlive();
}

void TbxShmStream::writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) {
    assert(addressSpace != AddressSpaceValues::TraceGttEntry);

    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
    for (auto &entry : writeInfoTable) {
        size_t destSize = 0;
        void *p = nullptr;

        translatePhysicalAddressToSystemMemory(entry.physicalAddress, sizeof(entry.tableEntry), isLocalMemory, p, destSize);
        memcpy_s(p, destSize, &entry.tableEntry, sizeof(entry.tableEntry));

        log << (isLocalMemory ? "Local" : "System") << " Mem: \t" << std::hex << entry.physicalAddress << " \t" << std::hex << entry.tableEntry
            << " \t" << std::hex << sizeof(entry.tableEntry) << std::endl;
    }
    checkSocketAlive();
}

void TbxShmStream::writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) {
    for (auto &entry : writeInfoTable) {
        size_t destSize = 0;
        void *p = nullptr;
        // GTT is storing a 0 based physical address natively so we can just write the entry as is
        // to the gttBaseAddress location
        translatePhysicalAddressToSystemMemory(ggtt->gttTableOffset + entry.physicalAddress, sizeof(entry.tableEntry), ggtt->isLocalMemory(), p, destSize);
        memcpy_s(p, destSize, &entry.tableEntry, sizeof(entry.tableEntry));
    }
    checkSocketAlive();
}

void TbxShmStream::checkSocketAlive() {
    std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();

    // If we poll the simulator everytime we read memory we will actually slow it down because
    // it task switch away to reply to us instead of computing results.
    if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTimeCheck).count() > 10) {
        socket->stillConnected();
        lastTimeCheck = currentTime;
    }
}

void TbxShmStream::enableThrowOnError(bool enabled) {
    socket->enableThrowOnError(enabled);
}

} // namespace aub_stream