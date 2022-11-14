/*
 * Copyright (C) 2022 Intel Corporation
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

bool TbxShmStream::init(int stepping, uint32_t device, CoreFamily gfxCoreFamily) {
    assert(0); // Shouldn't ever get here!
    return false;
}

bool TbxShmStream::init(int stepping, uint32_t device, CoreFamily gfxCoreFamily, SharedMemoryInfo *sharedMemoryInfo) {
    socket = TbxSockets::create();
    assert(socket != nullptr);

    this->sharedMemoryInfo = sharedMemoryInfo;

    return socket->init(tbxServerIp, tbxServerPort, tbxFrontdoorMode);
}

void TbxShmStream::readContiguousPages(void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;

    // read shared mem
    if (isLocalMemory) {
        memcpy_s(memory, size, (void *)(sharedMemoryInfo->localMemBase + physAddress), size);
    } else {
        memcpy_s(memory, size, (void *)(sharedMemoryInfo->sysMemBase + physAddress), size);
    }
    checkSocketAlive();
}

void TbxShmStream::readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) {
    for (auto &entry : writeInfoTable) {

        // read shared mem
        if (entry.isLocalMemory) {
            memcpy_s(memory, size, (void *)(sharedMemoryInfo->localMemBase + entry.physicalAddress), entry.size);
        } else {
            memcpy_s(memory, size, (void *)(sharedMemoryInfo->sysMemBase + entry.physicalAddress), entry.size);
        }

        memory = entry.size + (uint8_t *)memory;
    }
    checkSocketAlive();
}

void TbxShmStream::registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) {
    bool matches = false;
    do {
        uint32_t value;
        socket->readMMIO(registerOffset, &value);

        matches = ((value & mask) == desiredValue);
    } while (matches == pollNotEqual);
}

void TbxShmStream::writeMMIO(uint32_t offset, uint32_t value) {
    log << "MMIO: " << std::hex << std::showbase << offset
        << "   =: " << std::hex << std::showbase << value;

    socket->writeMMIO(offset, value);
}

void TbxShmStream::writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;

    if (isLocalMemory) {
        memcpy_s((void *)(sharedMemoryInfo->localMemBase + physAddress), static_cast<size_t>(sharedMemoryInfo->localMemSize - physAddress), memory, size);
        log << "Local Mem: \t" << std::hex << physAddress << " \t" << std::hex << "0x00000000"
            << " \t" << std::hex << size << std::endl;
    } else {
        memcpy_s((void *)(sharedMemoryInfo->sysMemBase + physAddress), static_cast<size_t>(sharedMemoryInfo->sysMemSize - physAddress), memory, size);
        log << "System Mem: \t" << std::hex << physAddress << " \t" << std::hex << "0x00000000"
            << " \t" << std::hex << size << std::endl;
    }
    checkSocketAlive();
}

void TbxShmStream::writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) {
    for (auto &entry : writeInfoTable) {
        if (entry.isLocalMemory) {
            memcpy_s((void *)(sharedMemoryInfo->localMemBase + entry.physicalAddress), static_cast<size_t>(sharedMemoryInfo->localMemSize - entry.physicalAddress), memory, entry.size);
            log << "Local Mem: \t" << std::hex << entry.physicalAddress << " \t" << std::hex << "0x00000000"
                << " \t" << std::hex << entry.size << std::endl;
        } else {
            memcpy_s((void *)(sharedMemoryInfo->sysMemBase + entry.physicalAddress), static_cast<size_t>(sharedMemoryInfo->sysMemSize - entry.physicalAddress), memory, entry.size);
            log << "System Mem: \t" << std::hex << entry.physicalAddress << " \t" << std::hex << "0x00000000"
                << " \t" << std::hex << entry.size << std::endl;
        }

        memory = entry.size + (uint8_t *)memory;
    }
    checkSocketAlive();
}

void TbxShmStream::writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) {
    assert(addressSpace != AddressSpaceValues::TraceGttEntry);

    bool isLocalMemory = addressSpace == AddressSpaceValues::TraceLocal;
    for (auto &entry : writeInfoTable) {
        if (isLocalMemory) {
            memcpy_s((void *)(sharedMemoryInfo->localMemBase + entry.physicalAddress), static_cast<size_t>(sharedMemoryInfo->localMemSize - entry.physicalAddress), &entry.tableEntry, sizeof(entry.tableEntry));
            log << "Local Mem: \t" << std::hex << entry.physicalAddress << " \t" << std::hex << entry.tableEntry << " \t" << std::hex << sizeof(entry.tableEntry) << std::endl;
        } else {
            memcpy_s((void *)(sharedMemoryInfo->sysMemBase + entry.physicalAddress), static_cast<size_t>(sharedMemoryInfo->sysMemSize - entry.physicalAddress), &entry.tableEntry, sizeof(entry.tableEntry));
            log << "System Mem: \t" << std::hex << entry.physicalAddress << " \t" << std::hex << entry.tableEntry << " \t" << std::hex << sizeof(entry.tableEntry) << std::endl;
        }
    }
    checkSocketAlive();
}

void TbxShmStream::writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) {
    uint8_t *gttBaseAddress = nullptr;
    size_t destSize = 0;

    if (this->sharedMemoryInfo->localMemBase != nullptr) {
        gttBaseAddress = this->sharedMemoryInfo->localMemBase + ggtt->gttTableOffset;
        destSize = static_cast<size_t>(this->sharedMemoryInfo->localMemSize - ggtt->gttTableOffset);
    } else {
        gttBaseAddress = this->sharedMemoryInfo->sysMemBase + ggtt->gttTableOffset;
        destSize = static_cast<size_t>(this->sharedMemoryInfo->sysMemSize - ggtt->gttTableOffset);
    }

    for (auto &entry : writeInfoTable) {
        // GTT is storing a 0 based physical address natively so we can just write the entry as is
        // to the gttBaseAddress location
        memcpy_s((void *)(gttBaseAddress + entry.physicalAddress), destSize, &entry.tableEntry, sizeof(entry.tableEntry));
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