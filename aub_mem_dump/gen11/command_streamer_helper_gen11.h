/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/options.h"

namespace aub_stream {

template <typename Helper>
struct CommandStreamerHelperGen11 : public Helper {
    using Helper::Helper;

    void submitContext(AubStream &stream, MiContextDescriptorReg &contextDescriptor) const override {
        stream.writeMMIO(mmioEngine + 0x2510, contextDescriptor.ulData[0]);
        stream.writeMMIO(mmioEngine + 0x2514, contextDescriptor.ulData[1]);

        // Load our new exec list
        stream.writeMMIO(mmioEngine + 0x2550, 1);
    }

    const uint32_t getPollForCompletionMask() const override { return 0x00008000; }

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuGen11 : public Gpu {
    CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const override;

    const std::vector<EngineType> getSupportedEngines() const override {
        static constexpr std::array<EngineType, 4> engines = {{ENGINE_RCS, ENGINE_BCS, ENGINE_VCS, ENGINE_VECS}};
        return std::vector<EngineType>(engines.begin(), engines.end());
    }

    const MMIOList getGlobalMMIO() const override {
        const MMIOList globalMMIO = {
            // GFX_MOCS_0 - GFX_MOCS_11
            MMIOPair(0x0000C800, 0x00000009),
            MMIOPair(0x0000C804, 0x00000038),
            MMIOPair(0x0000C808, 0x0000003B),
            MMIOPair(0x0000C80C, 0x00000039),
            MMIOPair(0x0000C810, 0x00000037),
            MMIOPair(0x0000C814, 0x00000039),
            MMIOPair(0x0000C818, 0x00000037),
            MMIOPair(0x0000C81C, 0x0000001B),
            MMIOPair(0x0000C820, 0x00060037),
            MMIOPair(0x0000C824, 0x00000032),
            MMIOPair(0x0000C828, 0x00000033),
            MMIOPair(0x0000C82C, 0x0000003B),
            MMIOPair(0x0000C8C0, 0x00000037),
            // SAMPLER_MODE
            MMIOPair(0x0000E18C, 0x00200020),
        };

        return globalMMIO;
    }

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override {
        return memoryBanks == MEMORY_BANK_SYSTEM && (alignment == 4096 || alignment == 65536);
    }

    void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override {
    }
    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override {
    }
    uint64_t getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize) const override {
        return 0;
    }
};
} // namespace aub_stream
