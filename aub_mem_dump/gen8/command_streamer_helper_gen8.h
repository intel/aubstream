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
struct CommandStreamerHelperGen8 : public Helper {
    using Helper::Helper;

    void submitContext(AubStream &stream, MiContextDescriptorReg &contextDescriptor) const override {
        stream.writeMMIO(mmioEngine + 0x2230, 0);
        stream.writeMMIO(mmioEngine + 0x2230, 0);

        stream.writeMMIO(mmioEngine + 0x2230, contextDescriptor.ulData[1]);
        stream.writeMMIO(mmioEngine + 0x2230, contextDescriptor.ulData[0]);
    }

    const MMIOList getEngineMMIO() const override;

    using Helper::mmioDevice;
    using Helper::mmioEngine;
};

struct GpuGen8 : public Gpu {
    CommandStreamerHelper &getCommandStreamerHelper(uint32_t device, EngineType engineType) const override;

    const std::vector<EngineType> getSupportedEngines() const override {
        constexpr std::array<EngineType, 4> engines = {{ENGINE_RCS, ENGINE_BCS, ENGINE_VCS, ENGINE_VECS}};
        return std::vector<EngineType>(engines.begin(), engines.end());
    }

    const MMIOList getGlobalMMIO() const override {
        const MMIOList globalMMIO = {};

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
