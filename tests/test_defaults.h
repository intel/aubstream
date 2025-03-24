/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include "gfx_core_family.h"
#include "aubstream/product_family.h"
#include "aub_mem_dump/gpu.h"
#include <memory>
#include <functional>

namespace aub_stream {
struct TestTraits;

constexpr uint64_t maxNBitValue(uint32_t N) {
    return (1ull << N) - 1;
}

constexpr uint64_t gpuAddressSpace48 = maxNBitValue(48);

extern std::unique_ptr<Gpu> gpu;
extern std::function<std::unique_ptr<Gpu>()> createGpuFunc;

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
std::string getAubConfigWithTile(const TestTraits *traits, const GpuDescriptor &desc);
std::string getAubConfigWithoutTile(const TestTraits *traits, const GpuDescriptor &desc);
std::string getAubFileName(const GpuDescriptor &desc);

bool initializeAubStream(AubFileStream &stream);
bool initializeAubStream(AubFileStream &stream,
                         const GpuDescriptor &desc);

} // namespace aub_stream
