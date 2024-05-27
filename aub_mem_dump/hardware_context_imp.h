/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aubstream/hardware_context.h"
#include "aubstream/engine_node.h"
#include "page_table.h"
#include <type_traits>
#include <vector>

namespace aub_stream {
struct AubStream;
struct CommandStreamerHelper;
struct HardwareContextImp;

struct ContextGroup {
    uint32_t contextGroupCounter = 0;
    std::vector<HardwareContextImp *> contexts = {};
};

struct GroupContextHelper {
    // Per device, per Engine (engine offset)
    ContextGroup contextGroups[4][EngineType::NUM_ENGINES];
};

struct HardwareContextImp : public HardwareContext {
    static constexpr uint64_t priorityLow = 0;
    static constexpr uint32_t priorityNormal = 1;
    static constexpr uint64_t priorityHigh = 2;

    uint32_t contextGroupId = -1;
    uint32_t priority = priorityNormal;

    HardwareContextImp(uint32_t deviceIndex, AubStream &aubStream, const CommandStreamerHelper &traits, GGTT &ggtt, PageTable &ppgtt, uint32_t flags) : HardwareContextImp(deviceIndex, aubStream, traits, ggtt, ppgtt, nullptr, flags){};

    HardwareContextImp(uint32_t deviceIndex, AubStream &aubStream, const CommandStreamerHelper &traits, GGTT &ggtt, PageTable &ppgtt, GroupContextHelper *groupHelper, uint32_t flags);

    ~HardwareContextImp() override;
    HardwareContextImp(const HardwareContextImp &) = delete;
    HardwareContextImp &operator=(const HardwareContextImp &) = delete;

    void initialize() override;
    void release() override;
    void pollForCompletion() override;
    void writeAndSubmitBatchBuffer(uint64_t gfxAddress, const void *batchBuffer, size_t size, uint32_t memoryBanks, size_t pageSize) override;
    void writeMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) override;
    void freeMemory(uint64_t gfxAddress, size_t size) override;
    void expectMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t compareOperation) override;
    void readMemory(uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize) override;
    void readGttMemory(uint64_t gfxAddress, void *memory, size_t size, uint32_t memoryBanks, size_t pageSize);
    void dumpBufferBIN(uint64_t gfxAddress, size_t size) override;
    void dumpSurface(const SurfaceInfo &surfaceInfo) override;
    void submitBatchBuffer(uint64_t gfxAddress, bool overrideRingHead) override;
    void writeMemory2(AllocationParams allocationParams) override;
    void writeMMIO(uint32_t offset, uint32_t value) override;
    void pollForFenceCompletion() override;

    AubStream &stream;
    const CommandStreamerHelper &csTraits;
    uint32_t deviceIndex;
    GGTT &ggtt;
    PageTable &ppgtt;
    uint32_t ggttRing;

    uint32_t ringData;
    uint8_t *pRingData;

    uint32_t ggttLRCA;
    uint32_t ggttGlobalHWSP;
    uint8_t *pLRCA;
    uint32_t ringTail;
    size_t ringSize;
    uint32_t flags;
    uint32_t ggttContextFence;
    uint32_t contextFenceValue;
    static uint32_t globalContextId;
    uint32_t contextId{globalContextId++};

    GroupContextHelper *contextGroupHelper;
};

} // namespace aub_stream
