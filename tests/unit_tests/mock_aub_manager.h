/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aub_mem_dump/memory_banks.h"
#include "aubstream/aubstream.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include <string>

namespace aub_stream {

class MockAubManager : public AubManagerImp {
  public:
    using AubManagerImp::AubManagerImp;
    using AubManagerImp::getStream;
    using AubManagerImp::ggtts;
    using AubManagerImp::hwContexts;
    using AubManagerImp::physicalAddressAllocator;
    using AubManagerImp::ppgtts;
    using AubManagerImp::stepping;
    using AubManagerImp::streamAub;
    using AubManagerImp::streamAubTbx;
    using AubManagerImp::streamTbx;
    using AubManagerImp::streamTbxShm;

    MockAubManager(std::unique_ptr<Gpu> gpu, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t streamMode)
        : AubManagerImp(std::move(gpu), {/* version */ 0, /* Product Family not used*/ 0, devicesCount, memoryBankSize, stepping, localMemorySupported, streamMode, gpuAddressSpace48, {}, {}, 4 * MB}) {}

    MockAubManager(std::unique_ptr<Gpu> gpu, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t streamMode, const SharedMemoryInfo &sharedMemoryInfo)
        : AubManagerImp(std::move(gpu), {/* version */ 0, /* Product Family not used*/ 0, devicesCount, memoryBankSize, stepping, localMemorySupported, streamMode, gpuAddressSpace48, sharedMemoryInfo, {}, 4 * MB}) {}

    virtual void createStream() override {
        if (streamCreated) {
            return;
        }

        if (createMockAubFileStream && streamMode == aub_stream::mode::aubFile) {
            mockAubFileStream = new MockAubFileStream();
            streamAub.reset(mockAubFileStream);
        } else if (streamMode == aub_stream::mode::tbx) {
            streamTbx = std::make_unique<MockReadMMIOTbxStream>();
            uint32_t localMemDevicesCount = 0;
            for (uint32_t i = 0; i < devicesCount; i++) {
                localMemDevicesCount += gpu->isMemorySupported(MEMORY_BANK_0 << i, 0x10000) ? 1 : 0;
            }
            if (localMemDevicesCount == 0) {
                EXPECT_CALL(*static_cast<MockReadMMIOTbxStream *>(streamTbx.get()), readMMIO(0x9118)).Times(0);
            } else {
                ON_CALL(*static_cast<MockReadMMIOTbxStream *>(streamTbx.get()), readMMIO(0x9118)).WillByDefault(::testing::Return(0x10000));
            }
        } else if (streamMode == aub_stream::mode::tbxShm || streamMode == aub_stream::mode::tbxShm4) {
            uint32_t localMemDevicesCount = 0;
            for (uint32_t i = 0; i < devicesCount; i++) {
                localMemDevicesCount += gpu->isMemorySupported(MEMORY_BANK_0 << i, 0x10000) ? 1 : 0;
            }
            streamTbxShm = std::make_unique<MockReadMMIOTbxShmStream>(streamMode);
            if (localMemDevicesCount == 0) {
                EXPECT_CALL(*static_cast<MockReadMMIOTbxShmStream *>(streamTbxShm.get()), readMMIO(0x9118)).Times(0);
            } else {
                ON_CALL(*static_cast<MockReadMMIOTbxShmStream *>(streamTbxShm.get()), readMMIO(0x9118)).WillByDefault(::testing::Return(0x10000));
            }
        } else if (streamMode == aub_stream::mode::aubFileAndTbx) {
            uint32_t localMemDevicesCount = 0;
            for (uint32_t i = 0; i < devicesCount; i++) {
                localMemDevicesCount += gpu->isMemorySupported(MEMORY_BANK_0 << i, 0x10000) ? 1 : 0;
            }
            streamTbx = std::make_unique<MockReadMMIOTbxStream>();
            streamAub = std::make_unique<AubFileStream>();
            streamAubTbx = std::make_unique<AubTbxStream>(*streamAub, *streamTbx);
            if (localMemDevicesCount == 0) {
                EXPECT_CALL(*static_cast<MockReadMMIOTbxStream *>(streamTbx.get()), readMMIO(0x9118)).Times(0);
            } else {
                ON_CALL(*static_cast<MockReadMMIOTbxStream *>(streamTbx.get()), readMMIO(0x9118)).WillByDefault(::testing::Return(0x10000));
            }
        } else {
            AubManagerImp::createStream();
        }
        streamCreated = true;
    }

    MockAubFileStream *getMockAubFileStream() const {
        return mockAubFileStream;
    }
    bool createMockAubFileStream = false;
    MockAubFileStream *mockAubFileStream = nullptr;
    bool streamCreated = false;
};

} // namespace aub_stream
