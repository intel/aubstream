/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_stream.h"
#include "headers/aubstream.h"
#include "gfx_core_family.h"
#include "test_defaults.h"
#include "gmock/gmock.h"

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

    MockAubManager(const Gpu &gpu, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t streamMode)
        : AubManagerImp(gpu, {/* Product Family not used*/ 0, devicesCount, memoryBankSize, stepping, localMemorySupported, streamMode, gpuAddressSpace48}) {}
};

} // namespace aub_stream
