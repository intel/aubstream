/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include <atomic>
#include <vector>

namespace aub_stream {

uint32_t HardwareContextImp::globalContextId = 0;

HardwareContextImp::HardwareContextImp(uint32_t deviceIndex, AubStream &aubStream, const CommandStreamerHelper &traits, GGTT &ggttIN, PageTable &ppgttIN, GroupContextHelper *groupHelper, uint32_t flags)
    : stream(aubStream),
      csTraits(traits),
      deviceIndex(deviceIndex),
      ggtt(ggttIN),
      ppgtt(ppgttIN),
      ggttRing(-1),
      ringData(-1),
      pRingData(nullptr),
      ggttLRCA(-1),
      ggttGlobalHWSP(-1),
      pLRCA(nullptr),
      ringTail(0),
      ringSize(16384u),
      flags(flags),
      ggttContextFence(0),
      contextFenceValue(0),
      contextGroupHelper(groupHelper) {

    constexpr uint32_t contextGroupBit = hardwareContextFlags::contextGroup;

    if (flags & hardwareContextFlags::highPriority) {
        priority = priorityHigh;
        this->flags = flags & (~hardwareContextFlags::highPriority); // unset
    }

    if (flags & contextGroupBit) {
        auto &groupContext = contextGroupHelper->contextGroups[deviceIndex][csTraits.engineType];

        assert(groupContext.contextGroupCounter < static_cast<uint32_t>(groupContext.contexts.size()));

        this->contextGroupId = groupContext.contextGroupCounter;

        groupContext.contextGroupCounter++;

        this->flags = flags & (~contextGroupBit); // unset
    }
}

HardwareContextImp::~HardwareContextImp() {
    HardwareContextImp::release();
}

void HardwareContextImp::initialize() {
    if (pLRCA) {
        return;
    }

    auto mmioEngine = csTraits.mmioEngine;
    auto &allocator = ggtt.gfxAddressAllocator;
    size_t pageSize = csTraits.isMemorySupported(ggtt.getMemoryBank(), 65536u)
                          ? 65536u
                          : 4096u;
    auto alignment = static_cast<uint32_t>(pageSize);

    csTraits.initializeEngineMMIO(stream);

    // Initialize global HWSP
    const size_t sizeHWSP = 4096;
    ggttGlobalHWSP = allocator.alignedAlloc(sizeHWSP, alignment);

    stream.writeMemory(
        &ggtt,
        ggttGlobalHWSP,
        nullptr,
        sizeHWSP,
        ggtt.getMemoryBank(),
        DataTypeHintValues::TraceNotype,
        pageSize);

    ggttContextFence = allocator.alignedAlloc(sizeHWSP, alignment);

    stream.writeMemory(
        &ggtt,
        ggttContextFence,
        &contextFenceValue,
        sizeof(contextFenceValue),
        ggtt.getMemoryBank(),
        DataTypeHintValues::TraceNotype,
        pageSize);

    stream.writeMMIO(mmioEngine + 0x2080, ggttGlobalHWSP);

    // Initialize ring buffer
    ggttRing = allocator.alignedAlloc(ringSize, alignment);
    stream.writeMemory(
        &ggtt,
        ggttRing,
        nullptr,
        ringSize,
        ggtt.getMemoryBank(),
        DataTypeHintValues::TraceNotype,
        pageSize);

    // Initialize LRCA
    auto sizeLRCA = csTraits.sizeLRCA;

    pLRCA = new uint8_t[sizeLRCA];
    auto ringCtrl = static_cast<uint32_t>((ringSize - 0x1000) | 1);
    csTraits.initialize(pLRCA, &ppgtt, flags, this->contextGroupId != -1);
    csTraits.setRingHead(pLRCA, 0x0000);
    csTraits.setRingTail(pLRCA, 0x0000);
    csTraits.setRingBase(pLRCA, ggttRing);
    csTraits.setRingCtrl(pLRCA, ringCtrl);

    ggttLRCA = allocator.alignedAlloc(sizeLRCA, alignment);

    if (csTraits.isRingDataEnabled()) {
        const size_t sizeRingData = 4096;
        ringData = allocator.alignedAlloc(sizeRingData, alignment);
        pRingData = new uint8_t[sizeRingData];

        csTraits.initializeRingData(pLRCA, pRingData, ringData, sizeRingData, ggttRing, ringCtrl);

        stream.writeMemory(
            &ggtt,
            ringData,
            pRingData,
            sizeRingData,
            ggtt.getMemoryBank(),
            csTraits.aubHintLRCA,
            pageSize);
    }

    stream.writeMemory(
        &ggtt,
        ggttLRCA,
        pLRCA,
        sizeLRCA,
        ggtt.getMemoryBank(),
        csTraits.aubHintLRCA,
        pageSize);

    stream.declareContextForDumping(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this)), &ppgtt);

    if (this->contextGroupId != -1) {
        auto &groupContextHelper = contextGroupHelper->contextGroups[deviceIndex][csTraits.engineType];

        assert(groupContextHelper.contexts[this->contextGroupId] == nullptr);

        groupContextHelper.contexts[this->contextGroupId] = this;
    }
}

void HardwareContextImp::release() {
    delete[] pLRCA;
    pLRCA = nullptr;

    delete[] pRingData;
    if (this->contextGroupId != -1) {
        auto &groupContextHelper = contextGroupHelper->contextGroups[deviceIndex][csTraits.engineType];

        groupContextHelper.contexts[this->contextGroupId] = nullptr;

        if (this->contextGroupId == 0) {
            for (uint32_t i = 0; i < groupContextHelper.contexts.size(); i++) {
                if (groupContextHelper.contexts[i]) {
                    groupContextHelper.contexts[i]->contextGroupId = -1;
                    groupContextHelper.contexts[i] = nullptr;
                }
            }
            groupContextHelper.contextGroupCounter = 0;
        }
    }
}

void HardwareContextImp::pollForCompletion() {
    csTraits.pollForCompletion(stream);
}

void HardwareContextImp::pollForFenceCompletion() {
    uint32_t currentValue = 0;
    size_t pageSize = csTraits.isMemorySupported(ggtt.getMemoryBank(), 65536u)
                          ? 65536u
                          : 4096u;

    do {
        std::atomic_thread_fence(std::memory_order_acquire);
        stream.readMemory(&ggtt, ggttContextFence, &currentValue, sizeof(currentValue), ggtt.getMemoryBank(), pageSize);
    } while (currentValue != contextFenceValue);
}

void HardwareContextImp::writeAndSubmitBatchBuffer(uint64_t gfxAddress, const void *batchBuffer, size_t size, uint32_t memoryBanks, size_t pageSize) {
    if (!csTraits.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize))) {
        pageSize = pageSize == 65536 ? 4096 : 65536;
    }
    assert(csTraits.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize)));

    writeMemory(
        gfxAddress,
        batchBuffer,
        size,
        memoryBanks,
        csTraits.aubHintBatchBuffer,
        pageSize);

    submitBatchBuffer(gfxAddress, false);
}

void HardwareContextImp::submitBatchBuffer(uint64_t gfxAddress, bool overrideRingHead) {
    contextFenceValue++;

    // Submit a batch buffer
    std::vector<uint32_t> ringCommands;

    csTraits.addBatchBufferJump(ringCommands, gfxAddress, this->contextGroupId != -1);
    csTraits.addFlushCommands(ringCommands);
    csTraits.storeFenceValue(ringCommands, ggttContextFence, contextFenceValue);

    if (ringCommands.size() % 2) {
        ringCommands.push_back(0);
    }

    size_t pageSize = csTraits.isMemorySupported(ggtt.getMemoryBank(), 65536u)
                          ? 65536u
                          : 4096u;

    const uint32_t sizeCommandsInBytes = static_cast<uint32_t>(ringCommands.size() * sizeof(uint32_t));

    if ((ringTail + sizeCommandsInBytes) >= ringSize) {
        size_t sizeToNoop = ringSize - ringTail;
        std::vector<uint32_t> noops;
        noops.insert(noops.begin(), sizeToNoop, 0);

        stream.writeMemory(
            &ggtt,
            ggttRing + ringTail,
            noops.data(),
            sizeToNoop,
            ggtt.getMemoryBank(),
            csTraits.aubHintCommandBuffer,
            pageSize);

        ringTail = 0;
    }

    stream.writeMemory(
        &ggtt,
        ggttRing + ringTail,
        ringCommands.data(),
        sizeCommandsInBytes,
        ggtt.getMemoryBank(),
        csTraits.aubHintCommandBuffer,
        pageSize);

    auto ringHead = ringTail;
    ringTail += sizeCommandsInBytes;

    auto ringOffset = csTraits.offsetContext + csTraits.offsetRingRegisters;
    auto ringDataOffset = csTraits.getRingDataOffset();
    auto size = 0u;
    if (overrideRingHead) {
        // Update the LRCA with the ring buffer head and tail
        csTraits.setRingHead(pLRCA, ringHead);
        csTraits.setRingTail(pLRCA, ringTail);
        ringOffset += csTraits.offsetRingHead;

        csTraits.setRingDataHead(pRingData, ringHead);
        csTraits.setRingDataTail(pRingData, ringTail);
        ringDataOffset += csTraits.offsetRingHead;

        size = 4 * sizeof(uint32_t);
    } else {
        // Update the LRCA with the ring buffer tail
        csTraits.setRingTail(pLRCA, ringTail);
        ringOffset += csTraits.offsetRingTail;

        csTraits.setRingDataTail(pRingData, ringTail);
        ringDataOffset += csTraits.offsetRingTail;

        size = 2 * sizeof(uint32_t);
    }

    stream.writeMemory(
        &ggtt,
        ggttLRCA + ringOffset,
        pLRCA + ringOffset,
        size,
        ggtt.getMemoryBank(),
        DataTypeHintValues::TraceNotype,
        pageSize);

    if (csTraits.isRingDataEnabled()) {
        stream.writeMemory(
            &ggtt,
            ringData + ringDataOffset,
            pRingData + ringDataOffset,
            size,
            ggtt.getMemoryBank(),
            DataTypeHintValues::TraceNotype,
            pageSize);
    }

    if (this->contextGroupId != -1) {
        auto &groupContextHelper = contextGroupHelper->contextGroups[deviceIndex][csTraits.engineType];

        csTraits.submit(stream, groupContextHelper.contexts, ppgtt.getNumAddressBits() != 32);
    } else {
        csTraits.submit(stream, ggttLRCA, ppgtt.getNumAddressBits() != 32, contextId, priority);
    }
}

void HardwareContextImp::writeMemory2(AllocationParams allocationParams) {
    auto &pageSize = allocationParams.pageSize;
    auto memoryBanks = allocationParams.memoryBanks;

    if (!csTraits.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize))) {
        pageSize = pageSize == 65536 ? 4096 : 65536;
    }
    assert(csTraits.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize)));

    stream.writeMemory(&ppgtt, allocationParams);
}

void HardwareContextImp::writeMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) {
    writeMemory2({gfxAddress, memory, size, memoryBanks, hint, pageSize}); // fallback to new interface
}

void HardwareContextImp::freeMemory(uint64_t gfxAddress, size_t size) {
    stream.freeMemory(&ppgtt,
                      gfxAddress,
                      size);
}

void HardwareContextImp::expectMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation) {
    stream.expectMemory(
        &ppgtt,
        gfxAddress,
        memory,
        size,
        compareOperation);
}

void HardwareContextImp::readMemory(uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize) {
    stream.readMemory(
        &ppgtt,
        gfxAddress,
        memory,
        size,
        memoryBanks,
        pageSize);
}

void HardwareContextImp::writeMMIO(uint32_t offset, uint32_t value) {
    stream.writeMMIO(
        offset,
        value);
}

void HardwareContextImp::dumpBufferBIN(uint64_t gfxAddress, size_t size) {
    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress, size, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this)));
}

void HardwareContextImp::dumpSurface(const SurfaceInfo &surfaceInfo) {
    stream.dumpSurface(AubStream::PAGE_TABLE_PPGTT, surfaceInfo, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this)));
}

} // namespace aub_stream
