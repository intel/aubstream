/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include "gfx_core_family.h"
#include "aubstream/headers/product_family.h"
#include "aub_mem_dump/gpu.h"

namespace aub_stream {

constexpr uint64_t maxNBitValue(uint32_t N) {
    return (1ull << N) - 1;
}

constexpr uint64_t gpuAddressSpace48 = maxNBitValue(48);

extern const Gpu *gpu;

extern uint32_t defaultDevice;
extern uint32_t defaultStepping;
extern uint32_t defaultDeviceCount;
extern size_t defaultPageSize;
extern size_t defaultHBMSizePerDevice;
extern uint32_t defaultMemoryBank;
extern uint32_t systemMemoryBank;
extern EngineType defaultEngine;
extern const bool localMemorySupportedInTests;

extern std::string folderAUB;
extern std::string fileSeparator;

struct AubFileStream;

std::string getAubFileName(const GpuDescriptor &desc);

bool initializeAubStream(AubFileStream &stream);
bool initializeAubStream(AubFileStream &stream,
                         const GpuDescriptor &desc);

} // namespace aub_stream
