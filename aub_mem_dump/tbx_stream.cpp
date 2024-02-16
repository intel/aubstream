/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tbx_stream.h"
#include "tbx_sockets.h"
#include "options.h"
#include <cassert>

namespace aub_stream {

TbxStream::~TbxStream() {
    if (socket) {
        socket->close();
        delete socket;
    }
}

void TbxStream::addComment(const char *message) {
    log << message;
}

void TbxStream::expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, uint32_t compareOperation) {
}

void TbxStream::declareContextForDumping(uint32_t handleDumpContext, PageTable *pageTable) {
}

void TbxStream::dumpBufferBIN(PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) {
}

void TbxStream::dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) {
}

void TbxStream::reserveContiguousPages(const std::vector<uint64_t> &entries) {
}

bool TbxStream::init(int stepping, const GpuDescriptor &gpu) {
    socket = TbxSockets::create();
    assert(socket != nullptr);
    return socket->init(tbxServerIp, tbxServerPort, tbxFrontdoorMode);
}

void TbxStream::readContiguousPages(void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
    socket->readMemory(physAddress, memory, size, isLocalMemory);
}

void TbxStream::readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) {
    for (auto &entry : writeInfoTable) {
        socket->readMemory(entry.physicalAddress, memory, entry.size, entry.isLocalMemory);
        memory = entry.size + (uint8_t *)memory;
    }
}

void TbxStream::registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) {
    bool matches = false;
    do {
        uint32_t value = 0;
        auto status = socket->readMMIO(registerOffset, &value);

        if (!status) {
            break;
        }

        matches = ((value & mask) == desiredValue);
        if (value & 1) {
            break;
        }
    } while (matches == pollNotEqual);
}

void TbxStream::writeMMIO(uint32_t offset, uint32_t value) {
    log << "MMIO: " << std::hex << std::showbase << offset
        << "   =: " << std::hex << std::showbase << value;

    socket->writeMMIO(offset, value);
}

void TbxStream::writePCICFG(uint32_t offset, uint32_t value) {
    socket->writePCICFG(0, 2, 0, offset, value);
}
uint32_t TbxStream::readPCICFG(uint32_t offset) {
    uint32_t value;
    socket->readPCICFG(0, 2, 0, offset, &value);
    return value;
}

uint32_t TbxStream::readMMIO(uint32_t offset) {
    uint32_t value;
    socket->readMMIO(offset, &value);
    return value;
}

void TbxStream::writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
    socket->writeMemory(physAddress, memory, size, isLocalMemory);
}

void TbxStream::writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) {
    for (auto &entry : writeInfoTable) {
        socket->writeMemory(entry.physicalAddress, memory, entry.size, entry.isLocalMemory);
        memory = entry.size + (uint8_t *)memory;
    }
}

void TbxStream::writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) {
    if (addressSpace == AddressSpaceValues::TraceGttEntry) {

        for (auto &entry : writeInfoTable) {
            socket->writeGTT(uint32_t(entry.physicalAddress), entry.tableEntry);
        }
    } else {
        bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
        for (auto &entry : writeInfoTable) {
            socket->writeMemory(entry.physicalAddress, &entry.tableEntry, sizeof(entry.tableEntry), isLocalMemory);
        }
    }
}

void TbxStream::writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) {
    writeDiscontiguousPages(writeInfoTable, AddressSpaceValues::TraceGttEntry, DataTypeHintValues::TraceNotype);
}

} // namespace aub_stream
