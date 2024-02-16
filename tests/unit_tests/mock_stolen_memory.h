/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/gpu.h"

namespace aub_stream {

struct MockStolenMemoryInStaticStorage : public StolenMemoryInStaticStorage {
    MockStolenMemoryInStaticStorage(uint64_t memoryBankSize, uint64_t dataStolenMemorySize) : StolenMemoryInStaticStorage(memoryBankSize, dataStolenMemorySize) {
    }
    using StolenMemoryInStaticStorage::staticMemoryBankSize;
};

struct MockStolenMemoryInHeap : public StolenMemoryInHeap {
    MockStolenMemoryInHeap(uint32_t deviceCount, uint64_t memoryBankSize, uint64_t dataStolenMemorySize) : StolenMemoryInHeap(deviceCount, memoryBankSize, dataStolenMemorySize) {
    }
    using StolenMemoryInHeap::localStolenStorage;
};

} // namespace aub_stream
