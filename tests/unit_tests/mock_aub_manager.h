/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aubstream/aubstream.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include <string>

namespace aub_stream {

class MockAubManager : public AubManagerImp {
  public:
    using AubManagerImp::AubManagerImp;
    using AubManagerImp::getStream;
    using AubManagerImp::ggtts;
    using AubManagerImp::physicalAddressAllocator;
    using AubManagerImp::ppgtts;
    using AubManagerImp::stepping;
    using AubManagerImp::streamAub;
    using AubManagerImp::streamAubTbx;
    using AubManagerImp::streamTbx;
    using AubManagerImp::streamTbxShm;

    MockAubManager(std::unique_ptr<Gpu> gpu, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t streamMode)
        : AubManagerImp(std::move(gpu), {/* version */ 0, /* Product Family not used*/ 0, devicesCount, memoryBankSize, stepping, localMemorySupported, streamMode, gpuAddressSpace48}) {}

    MockAubManager(std::unique_ptr<Gpu> gpu, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t streamMode, const SharedMemoryInfo &sharedMemoryInfo)
        : AubManagerImp(std::move(gpu), {/* version */ 0, /* Product Family not used*/ 0, devicesCount, memoryBankSize, stepping, localMemorySupported, streamMode, gpuAddressSpace48, sharedMemoryInfo}) {}

    virtual void createStream() override {
        if (gpu->productFamily <= ProductFamily::Arl) {
            AubManagerImp::createStream();
        } else if (streamMode == aub_stream::mode::tbx) {
            streamTbx = std::make_unique<MockReadMMIOTbxStream>();
            uint32_t localMemDevicesCount = 0;
            for (uint32_t i = 0; i < devicesCount; i++) {
                localMemDevicesCount = gpu->isMemorySupported(MEMORY_BANK_0 << i, 0x10000) ? 1 : 0;
            }
            EXPECT_CALL(*static_cast<MockReadMMIOTbxStream *>(streamTbx.get()), readMMIO(0x9118)).Times(localMemDevicesCount).WillRepeatedly(::testing::Return(1));
            ;
        } else if (streamMode == aub_stream::mode::tbxShm || streamMode == aub_stream::mode::tbxShm4) {
            uint32_t localMemDevicesCount = 0;
            for (uint32_t i = 0; i < devicesCount; i++) {
                localMemDevicesCount = gpu->isMemorySupported(MEMORY_BANK_0 << i, 0x10000) ? 1 : 0;
            }
            streamTbxShm = std::make_unique<MockReadMMIOTbxShmStream>(streamMode);
            EXPECT_CALL(*static_cast<MockReadMMIOTbxShmStream *>(streamTbxShm.get()), readMMIO(0x9118)).Times(localMemDevicesCount).WillRepeatedly(::testing::Return(1));
        } else if (streamMode == aub_stream::mode::aubFileAndTbx) {
            uint32_t localMemDevicesCount = 0;
            for (uint32_t i = 0; i < devicesCount; i++) {
                localMemDevicesCount = gpu->isMemorySupported(MEMORY_BANK_0 << i, 0x10000) ? 1 : 0;
            }
            streamTbx = std::make_unique<MockReadMMIOTbxStream>();
            streamAub = std::make_unique<AubFileStream>();
            streamAubTbx = std::make_unique<AubTbxStream>(*streamAub, *streamTbx);
            EXPECT_CALL(*static_cast<MockReadMMIOTbxStream *>(streamTbx.get()), readMMIO(0x9118)).Times(localMemDevicesCount).WillRepeatedly(::testing::Return(1));
        } else {
            AubManagerImp::createStream();
        }
    }
};

} // namespace aub_stream
