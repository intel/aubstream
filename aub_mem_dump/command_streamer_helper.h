/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/page_table.h"
#include "aub_services.h"
#include "aubstream/engine_node.h"
#include "aubstream/hint_values.h"
#include <cstdint>
#include <vector>
#include <mutex>

namespace aub_stream {

struct AubStream;
struct Gpu;
struct HardwareContextImp;

union MiContextDescriptorReg {
    struct {
        uint64_t Valid : 1;                  //[0]
        uint64_t ForcePageDirRestore : 1;    //[1]
        uint64_t ForceRestore : 1;           //[2]
        uint64_t Legacy : 1;                 //[3]
        uint64_t ADor64bitSupport : 1;       //[4] Selects 64-bit PPGTT in Legacy mode
        uint64_t LlcCoherencySupport : 1;    //[5]
        uint64_t FaultSupport : 2;           //[7:6]
        uint64_t PrivilegeAccessOrPPGTT : 1; //[8] Selects PPGTT in Legacy mode
        uint64_t FunctionType : 3;           //[11:9]
        uint64_t LogicalRingCtxAddress : 20; //[31:12]
        uint64_t Reserved : 7;               //[38:32]
        uint64_t ContextID : 16;             //[54:39]
        uint64_t Reserved2 : 9;              //[63:55]
    } sData;
    uint32_t ulData[2];
    uint64_t qwordData[2 / 2]{};
};

constexpr uint32_t ccsEngineOffset(uint32_t engineId) {
    if (engineId <= 2) {
        return 0x018000 + (engineId * 0x2000);
    } else if (engineId == 3) {
        return 0x024000;
    } else {
        return -1;
    }
};

constexpr uint32_t bcsLinkEngineOffset(uint32_t engineId) {
    if (engineId >= 1 && engineId <= 8) {
        return 0x3DC000 + (engineId * 0x2000);
    } else {
        return -1;
    }
};

struct CommandStreamerHelper {
    using AddressSpaceValues = CmdServicesMemTraceMemoryWrite::AddressSpaceValues;

    CommandStreamerHelper() = default;
    virtual ~CommandStreamerHelper() = default;
    CommandStreamerHelper(uint32_t deviceIndex, uint32_t offsetEngine) : deviceIndex(deviceIndex) {
        mmioDevice = deviceIndex * 0x1000000;
        mmioEngine = mmioDevice + offsetEngine;
    }
    std::string name = "XCS";

    const Gpu *gpu = nullptr;
    uint32_t deviceIndex = 0;
    size_t sizeLRCA = 0x2000;

    int aubHintLRCA = DataTypeHintValues::TraceNotype;
    int aubHintCommandBuffer = DataTypeHintValues::TraceCommandBuffer;
    int aubHintBatchBuffer = DataTypeHintValues::TraceBatchBuffer;

    EngineType engineType = EngineType::NUM_ENGINES;

    uint32_t mmioDevice = 0;
    uint32_t mmioEngine = 0;

    uint32_t alignLRCA = 0x1000;
    uint32_t offsetContext = 0x1000;

    uint32_t offsetLRI0 = 0x01 * sizeof(uint32_t);
    uint32_t numRegsLRI0 = 14;

    uint32_t numNoops0 = 3;

    uint32_t offsetLRI1 = offsetLRI0 + (1 + numRegsLRI0 * 2 + numNoops0) * sizeof(uint32_t); // offsetLRI == 0x21 * sizeof(uint32_t);
    uint32_t numRegsLRI1 = 9;

    uint32_t numNoops1 = 13;

    uint32_t offsetLRI2 = offsetLRI1 + (1 + numRegsLRI1 * 2 + numNoops1) * sizeof(uint32_t); // offsetLR2 == 0x41 * sizeof(uint32_t);
    uint32_t numRegsLRI2 = 1;

    uint32_t offsetRingRegisters = offsetLRI0 + (3 * sizeof(uint32_t));
    uint32_t offsetRingHead = 0x0 * sizeof(uint32_t);
    uint32_t offsetRingTail = 0x2 * sizeof(uint32_t);
    uint32_t offsetRingBase = 0x4 * sizeof(uint32_t);
    uint32_t offsetRingCtrl = 0x6 * sizeof(uint32_t);
    uint32_t offsetBbCurrentHeadReg = 0x8 * sizeof(uint32_t);

    uint32_t offsetPageTableRegisters = offsetLRI1 + (3 * sizeof(uint32_t));
    uint32_t offsetPDP0 = 0xc * sizeof(uint32_t);
    uint32_t offsetPDP1 = 0x8 * sizeof(uint32_t);
    uint32_t offsetPDP2 = 0x4 * sizeof(uint32_t);
    uint32_t offsetPDP3 = 0x0 * sizeof(uint32_t);

    void initialize(void *pLRCIn, PageTable *ppgtt, uint32_t flags) const;
    virtual void initializeContextEnvironment(AubStream &stream, HardwareContextImp &context) const {};
    virtual void cleanupContextEnvironment(AubStream &stream, HardwareContextImp &context) const {};
    bool isMemorySupported(uint32_t memoryBank, uint32_t alignment) const;
    size_t getSupportedPageSize(uint32_t memoryBank, size_t pageSize) const;
    void setRingHead(void *pLRCIn, uint32_t ringHead) const;
    void setRingTail(void *pLRCIn, uint32_t ringTail) const;
    void setRingBase(void *pLRCIn, uint32_t ringBase) const;
    void setRingCtrl(void *pLRCIn, uint32_t ringCtrl) const;
    void setBbCurrentHeadReg(void *pLRCIn) const;

    void setPDP0(void *pLRCIn, uint64_t address) const;
    void setPDP1(void *pLRCIn, uint64_t address) const;
    void setPDP2(void *pLRCIn, uint64_t address) const;
    void setPDP3(void *pLRCIn, uint64_t address) const;

    void setPML(void *pLRCIn, uint64_t address) const;

    virtual uint32_t getInitialContextSaveRestoreCtrlValue() const {
        return 0x00090009;
    }
    virtual uint32_t getRingContextSaveRestoreCtrlValue() const {
        return 0x00090008;
    }

    virtual void submit(AubStream &stream, uint32_t ggttLRCA, bool is48Bits, uint32_t contextId, uint32_t priority) const;
    virtual void submit(AubStream &stream, const std::vector<HardwareContextImp *> &hwContexts, bool is48Bits) const;
    virtual uint32_t getPollForCompletionMask() const { return 0x00000100; }
    void pollForCompletion(AubStream &stream) const;
    void initializeEngineMMIO(AubStream &stream) const;
    virtual const MMIOList getEngineMMIO() const = 0;

    virtual void addBatchBufferJump(std::vector<uint32_t> &ringBuffer, uint64_t gfxAddress) const;
    virtual void addFlushCommands(std::vector<uint32_t> &ringBuffer) const = 0;
    virtual void storeFenceValue(std::vector<uint32_t> &ringBuffer, uint64_t gfxAddress, uint32_t fenceValue) const;

    virtual bool isRingDataEnabled() const { return false; }
    virtual bool isExeclistSubmissionEnabled() const { return false; }
    virtual void initializeRingData(void *pLRCIn, void *ringState, uint32_t ringData, size_t sizeRingData, uint32_t ggttRing, uint32_t ringCtrl) const {}
    virtual uint32_t getRingDataOffset() const { return 0; }
    virtual void setRingDataHead(void *ringData, uint32_t ringHead) const {}
    virtual void setRingDataTail(void *ringData, uint32_t ringTail) const {}
    virtual uint32_t getHintForInRingCtx() const { return 0; }
    std::unique_lock<std::mutex> obtainUniqueLock() const {
        return std::unique_lock<std::mutex>(mutex);
    }
    virtual bool memoryBasedPollForCompletion() const { return false; }

  protected:
    virtual void submitContext(AubStream &stream, std::vector<MiContextDescriptorReg> &contextDescriptor) const = 0;
    virtual void setPriority(MiContextDescriptorReg &contextDescriptor, uint32_t priority) const {};
    mutable std::mutex mutex;
};

struct CommandStreamerHelperRcs : public CommandStreamerHelper {
    CommandStreamerHelperRcs(uint32_t deviceIndex) : CommandStreamerHelper(deviceIndex, 0x000000) {
        aubHintLRCA = DataTypeHintValues::TraceLogicalRingContextRcs;
        aubHintCommandBuffer = DataTypeHintValues::TraceCommandBufferPrimary;
        aubHintBatchBuffer = DataTypeHintValues::TraceBatchBufferPrimary;
        sizeLRCA = 0x11000;
        name = "RCS";

        engineType = EngineType::ENGINE_RCS;
    }

    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override {
        // Pipe Control for flushes
        ringBuffer.push_back(0x7a000004);
        ringBuffer.push_back(
            1 << 20 | // CSStallEnable
            1 << 12 | // RenderTargetCacheFlushEnable
            1 << 5 |  // DCFlushEnable
            1 << 9 |  // IndirectStatePointersDisable
            1 << 0    // DepthCacheFlushEnable
        );
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);

        // Pipe Control for Invalidate
        ringBuffer.push_back(0x7a000004);
        ringBuffer.push_back(
            1 << 20 | // CSStallEnable
            1 << 9 |  // IndirectStatePointersDisable
            1 << 2 |  // StateCacheInvalidationEnable
            1 << 3 |  // ConstantCacheInvalidationEnable
            1 << 4 |  // VFCacheInvalidationEnable
            1 << 5 |  // DCFlushEnable
            1 << 10 | // TextureCacheInvalidateEnable
            1 << 11 | // InstructionCacheInvalidateEnable
            1 << 29   // CommandCacheInvalidateEnable
        );
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
    }
};

struct CommandStreamerHelperBcs : public CommandStreamerHelper {
    CommandStreamerHelperBcs(uint32_t deviceIndex) : CommandStreamerHelper(deviceIndex, 0x020000) {
        aubHintLRCA = DataTypeHintValues::TraceLogicalRingContextBcs;
        aubHintCommandBuffer = DataTypeHintValues::TraceCommandBufferBlt;
        aubHintBatchBuffer = DataTypeHintValues::TraceBatchBufferBlt;
        name = "BCS";

        engineType = EngineType::ENGINE_BCS;
    }

    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override {
        // MI_FLUSH_DW
        ringBuffer.push_back(0x13000003);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
    }
};

struct CommandStreamerHelperVcs : public CommandStreamerHelper {
    CommandStreamerHelperVcs(uint32_t deviceIndex) : CommandStreamerHelper(deviceIndex, 0x1be000) {
        aubHintLRCA = DataTypeHintValues::TraceLogicalRingContextVcs;
        aubHintCommandBuffer = DataTypeHintValues::TraceCommandBufferMfx;
        aubHintBatchBuffer = DataTypeHintValues::TraceBatchBufferMfx;
        name = "VCS";

        engineType = EngineType::ENGINE_VCS;
    }

    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override {
        // MI_FLUSH_DW
        ringBuffer.push_back(0x13000003);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
    }
};

struct CommandStreamerHelperVecs : public CommandStreamerHelper {
    CommandStreamerHelperVecs(uint32_t deviceIndex) : CommandStreamerHelper(deviceIndex, 0x1c6000) {
        aubHintLRCA = DataTypeHintValues::TraceLogicalRingContextVecs;
        name = "VECS";

        engineType = EngineType::ENGINE_VECS;
    }

    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override {
        // MI_FLUSH_DW
        ringBuffer.push_back(0x13000003);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
    }
};

struct CommandStreamerHelperCcs : public CommandStreamerHelper {
    CommandStreamerHelperCcs(uint32_t deviceId, uint32_t engineId) : CommandStreamerHelper(deviceId,
                                                                                           ccsEngineOffset(engineId)) {
        aubHintLRCA = DataTypeHintValues::TraceLogicalRingContextCcs;
        name = "CCS";

        engineType = static_cast<EngineType>(EngineType::ENGINE_CCS + engineId);
    }

    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override {
        // Pipe Control for flushes
        ringBuffer.push_back(0x7a000004);
        ringBuffer.push_back(
            1 << 20 | // CSStallEnable
            1 << 5 |  // DCFlushEnable
            1 << 9    // IndirectStatePointersDisable
        );
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);

        // Pipe Control for Invalidate
        ringBuffer.push_back(0x7a000004);
        ringBuffer.push_back(
            1 << 20 | // CSStallEnable
            1 << 9 |  // IndirectStatePointersDisable
            1 << 2 |  // StateCacheInvalidationEnable
            1 << 3 |  // ConstantCacheInvalidationEnable
            1 << 5 |  // DCFlushEnable
            1 << 10 | // TextureCacheInvalidateEnable
            1 << 11 | // InstructionCacheInvalidateEnable
            1 << 29   // CommandCacheInvalidateEnable
        );
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
    }
};

struct CommandStreamerHelperCccs : public CommandStreamerHelperRcs {
    CommandStreamerHelperCccs(uint32_t baseDevice) : CommandStreamerHelperRcs(baseDevice) {
        name = "CCCS";

        engineType = EngineType::ENGINE_CCCS;
    }
};

struct CommandStreamerHelperLinkBcs : public CommandStreamerHelper {
    CommandStreamerHelperLinkBcs(uint32_t deviceIndex, uint32_t engineId) : CommandStreamerHelper(deviceIndex, bcsLinkEngineOffset(engineId)) {
        aubHintLRCA = DataTypeHintValues::TraceLogicalRingContextBcs;
        aubHintCommandBuffer = DataTypeHintValues::TraceCommandBufferBlt;
        aubHintBatchBuffer = DataTypeHintValues::TraceBatchBufferBlt;
        name = "BCS" + std::to_string(engineId);

        engineType = static_cast<EngineType>(EngineType::ENGINE_BCS1 + engineId - 1);
    }

    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override {
        // MI_FLUSH_DW
        ringBuffer.push_back(0x13000003);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
        ringBuffer.push_back(0);
    }
};

} // namespace aub_stream
