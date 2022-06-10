/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/page_table.h"
#include "gmock/gmock.h"

namespace aub_stream {

struct MockGpuBase : public Gpu {
    MOCK_CONST_METHOD4(initializeGlobalMMIO, void(AubStream &streamu, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping));
    MOCK_CONST_METHOD2(isMemorySupported, bool(uint32_t memoryBanks, uint32_t alignment));
    MOCK_CONST_METHOD1(isEngineSupported, bool(uint32_t engine));
    MOCK_CONST_METHOD3(setMemoryBankSize, void(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize));
    MOCK_CONST_METHOD3(setGGTTBaseAddresses, void(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize));
    MOCK_CONST_METHOD2(getCommandStreamerHelper, CommandStreamerHelper &(uint32_t device, EngineType engineType));
    MOCK_CONST_METHOD3(allocateGGTT, GGTT *(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gttBaseAddress));
    MOCK_CONST_METHOD3(allocatePPGTT, PageTable *(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace));
    MOCK_CONST_METHOD0(requireLocalMemoryForPageTables, bool());
    MOCK_CONST_METHOD0(getGlobalMMIO, const MMIOList());
    MOCK_CONST_METHOD0(getSupportedEngines, const std::vector<EngineType>());
    MOCK_CONST_METHOD2(getGGTTBaseAddress, uint64_t(uint32_t device, uint64_t memoryBankSize));
};

using MockGpu = ::testing::NiceMock<MockGpuBase>;

struct MockGpuFixture {
    void SetUp() {
    }

    void TearDown() {
    }

    MockGpu gpu;
};

} // namespace aub_stream
