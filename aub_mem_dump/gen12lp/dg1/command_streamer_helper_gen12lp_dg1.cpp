/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

namespace aub_stream {

struct GpuDg1 : public GpuGen12LP {
    GpuDg1() {
        productFamily = IGFX_DG1;
        gfxCoreFamily = GEN12LP_CORE;
        productAbbreviation = "dg1";
        deviceId = 0x1e;
        deviceCount = 1;
    }

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override {
        assert(MEMORY_BANK_SYSTEM == 0);
        auto supportedBanks = MEMORY_BANK_SYSTEM | MEMORY_BANK_0;
        auto unsupportedBanks = MEMORY_BANKS_ALL ^ supportedBanks;

        if (unsupportedBanks & memoryBanks) {
            return false;
        }

        // Local MEMORY_BANKs
        if (memoryBanks & supportedBanks) {
            return alignment == 4096 || alignment == 65536;
        }

        // MEMORY_BANK_SYSTEM
        return alignment == 4096 || alignment == 65536;
    }

    void setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override {
        assert(deviceCount == 1);
        assert(deviceCount <= this->deviceCount);

        auto gb = memoryBankSize / GB;
        assert(gb > 0);
        assert(gb < 128);

        size_t base = 0u;
        uint32_t offset = 0x4900;
        uint32_t value = 0;
        value |= gb << 8;
        value |= base << 1;
        value |= 1;

        stream.writeMMIO(offset, value);
    }
};

template <>
const Gpu *enableGpu<IGFX_DG1>() {
    static const GpuDg1 gpu;
    return &gpu;
}
} // namespace aub_stream
