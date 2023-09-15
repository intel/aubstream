/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_stream.h"
#include "aubstream/aubstream.h"

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
};

} // namespace aub_stream
