/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/xe_core/command_streamer_helper_xe_core.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/settings.h"

template <typename T>
inline T ptrOffset(T ptrBefore, size_t offset) {
    auto addrBefore = (uintptr_t)ptrBefore;
    auto addrAfter = addrBefore + offset;
    return (T)addrAfter;
}

namespace aub_stream {

template <typename Helper>
struct CommandStreamerHelperXe3pCore : public CommandStreamerHelperXeCore<Helper> {
    using CommandStreamerHelperXeCore<Helper>::CommandStreamerHelperXeCore;
    using BaseClass = CommandStreamerHelperXeCore<Helper>;

    void submitContext(AubStream &stream, std::vector<MiContextDescriptorReg> &contextDescriptor) const override {
        bool execlistSubmitPortEnabled = isExeclistSubmissionEnabled();

        if (!execlistSubmitPortEnabled) {

            for (uint32_t i = 0; i < 8; i++) {
                stream.writeMMIO(mmioEngine + 0x2510 + (i * 8), contextDescriptor[i].ulData[0]);
                stream.writeMMIO(mmioEngine + 0x2514 + (i * 8), contextDescriptor[i].ulData[1]);
            }
        } else {
            assert(contextDescriptor.size() <= 64);

            for (uint32_t i = 0; i < contextDescriptor.size(); i++) {
                // write lower DWORD first
                stream.writeMMIO(mmioEngine + 0x2230, contextDescriptor[i].ulData[0]);
                stream.writeMMIO(mmioEngine + 0x2230, contextDescriptor[i].ulData[1]);
            }
        }
        // Load our new exec list
        stream.writeMMIO(mmioEngine + 0x2550, 1);
    }

    uint32_t getInitialContextSaveRestoreCtrlValue() const override {

        uint32_t value = 0x00090009;
        value = isRingDataEnabled() ? value | (((1 << 4) << 16) | 1 << 4) : value; // Indirect Ring State Enable

        return value;
    }

    uint32_t getRingContextSaveRestoreCtrlValue() const override {

        uint32_t value = 0x00090008;
        value = isRingDataEnabled() ? value | (((1 << 4) << 16) | 1 << 4) : value; // Indirect Ring State Enable

        return value;
    }

    void initializeRingData(void *pLRCIn, void *state, uint32_t ringData, size_t sizeRingData, uint32_t ggttRing, uint32_t ringCtrl) const override {

        if (isRingDataEnabled()) {
            auto pLRCA = ptrOffset(reinterpret_cast<uint32_t *>(pLRCIn),
                                   Helper::offsetContext + Helper::offsetLRI1 + 5 * sizeof(uint32_t));

            *pLRCA++ = mmioEngine + 0x2108; // INDIRECT_RING_STATE
            *pLRCA++ = ringData;
        }

        auto ringState = reinterpret_cast<uint32_t *>(state);

        for (size_t i = 0; i < sizeRingData / sizeof(uint32_t); i++) {
            ringState[i] = 0x0;
        }

        // Initialize the ring state
        auto pLRI = ptrOffset(ringState, 1 * sizeof(uint32_t));
        auto numRegs = 5;
        *pLRI++ = 0x11001000 | (2 * numRegs - 1);

        auto regs = pLRI;
        while (numRegs-- > 0) {
            *regs++ = 0;
            *regs++ = 0x00000000;
        }

        auto pIndirectRingState = ptrOffset(reinterpret_cast<uint32_t *>(pLRI), Helper::offsetRingHead);

        *pIndirectRingState++ = mmioEngine + 0x2034;
        *pIndirectRingState++ = 0;

        pIndirectRingState = ptrOffset(reinterpret_cast<uint32_t *>(pLRI), Helper::offsetRingTail);
        *pIndirectRingState++ = mmioEngine + 0x2030;
        *pIndirectRingState++ = 0;

        pIndirectRingState = ptrOffset(reinterpret_cast<uint32_t *>(pLRI), Helper::offsetRingBase);
        *pIndirectRingState++ = mmioEngine + 0x2038;
        *pIndirectRingState++ = ggttRing;

        *pIndirectRingState++ = mmioEngine + 0x2048;
        *pIndirectRingState++ = 0;

        pIndirectRingState = ptrOffset(reinterpret_cast<uint32_t *>(pLRI), Helper::offsetRingCtrl + 2 * sizeof(uint32_t));
        *pIndirectRingState++ = mmioEngine + 0x203c;
        *pIndirectRingState++ = ringCtrl;

        pLRI = ptrOffset(ringState, 17 * sizeof(uint32_t));

        numRegs = 9;
        *pLRI++ = 0x11001000 | (2 * numRegs - 1);
    }

    bool isRingDataEnabled() const override {
        const bool indirectRingEnabled = globalSettings->IndirectRingState.get() != -1 ? globalSettings->IndirectRingState.get() : true;
        return indirectRingEnabled;
    }

    bool isExeclistSubmissionEnabled() const override {
        bool execlistSubmitPortEnabled = globalSettings->ExeclistSubmitPortSubmission.get() != -1 ? globalSettings->ExeclistSubmitPortSubmission.get() : true;
        return execlistSubmitPortEnabled;
    }

    uint32_t getRingDataOffset() const override {
        return 2 * sizeof(uint32_t);
    }

    void setRingDataHead(void *ringData, uint32_t ringHead) const override {
        if (ringData == nullptr) {
            return;
        }
        auto pIndirectRingState = ptrOffset(reinterpret_cast<uint32_t *>(ringData), getRingDataOffset() + Helper::offsetRingHead);
        *pIndirectRingState++ = mmioEngine + 0x2034;
        *pIndirectRingState++ = ringHead;
    }

    void setRingDataTail(void *ringData, uint32_t ringTail) const override {
        if (ringData == nullptr) {
            return;
        }
        auto pIndirectRingState = ptrOffset(reinterpret_cast<uint32_t *>(ringData), getRingDataOffset() + Helper::offsetRingTail);
        *pIndirectRingState++ = mmioEngine + 0x2030;
        *pIndirectRingState++ = ringTail;
    }

    using Helper::engineType;

    uint32_t getHintForInRingCtx() const override {
        uint32_t hint = 0;
        switch (engineType) {
        case EngineType::ENGINE_RCS:
            hint = DataTypeHintValues::TraceIndirectRingContextRcs;
            break;
        case EngineType::ENGINE_BCS:
            hint = DataTypeHintValues::TraceIndirectRingContextBcs;
            break;
        case EngineType::ENGINE_VCS:
            hint = DataTypeHintValues::TraceIndirectRingContextVcs;
            break;
        case EngineType::ENGINE_VECS:
            hint = DataTypeHintValues::TraceIndirectRingContextVecs;
            break;
        case EngineType::ENGINE_CCS:
            hint = DataTypeHintValues::TraceIndirectRingContextCcs;
            break;
        default:
            break;
        }
        return hint;
    }

    const MMIOList getEngineMMIO() const override;
    void addFlushCommands(std::vector<uint32_t> &ringBuffer) const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuXe3pCore : public GpuXeCore {
    GpuXe3pCore();
    static constexpr uint32_t numSupportedDevices = 4;
    static constexpr uint64_t patIndexBit0 = toBitValue(3);
    static constexpr uint64_t patIndexBit1 = toBitValue(4);
    static constexpr uint64_t patIndexBit2 = toBitValue(7);
    static constexpr uint64_t patIndexBit3 = toBitValue(62);
    static constexpr uint64_t patIndex0 = 0;                            // 0b0000
    static constexpr uint64_t patIndex3 = patIndexBit1 | patIndexBit0;  // 0b0011
    static constexpr uint64_t patIndex9 = patIndexBit3 | patIndexBit0;  // 0b1001
    static constexpr uint64_t patIndex12 = patIndexBit3 | patIndexBit2; // 0b1100

    CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const override;
    const std::vector<EngineType> getSupportedEngines() const override;

    static constexpr uint32_t getPatIndexMmioAddr(uint32_t index) {
        assert(index <= 31);
        uint32_t address = 0x4800 + (index * 4);

        if (index >= 8) {
            address += 0x28; // gap between 7 and 8
        }

        return address;
    }

    const MMIOList getGlobalMMIO() const override;
    void initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const override;

    uint64_t getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const override;
    PageTable *allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const override;
    bool isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const override;
    uint32_t sizeToGMS(uint64_t dataStolenMemorySize) const;
    uint32_t sizeToPAVPC(uint64_t wopcmMemorySize) const;
    void initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const override;
    virtual void initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress, uint64_t size) const {}
    void initializeTileRangeMmio(AubStream &stream, uint32_t deviceIndex, uint64_t lmemBaseAddress, uint64_t lmemSize) const;
    uint32_t getContextGroupCount() const override {
        return 64;
    }
    std::unique_ptr<CommandStreamerHelper> commandStreamerHelperTable[numSupportedDevices][EngineType::NUM_ENGINES];
};

} // namespace aub_stream
