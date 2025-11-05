/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/gpu.h"

namespace aub_stream {

struct MockStolenMemoryInStaticStorage : public StolenMemoryInStaticStorage {
    MockStolenMemoryInStaticStorage(uint64_t memoryBankSize, uint64_t size) : StolenMemoryInStaticStorage(memoryBankSize, size) {
    }
    using StolenMemoryInStaticStorage::staticMemoryBankSize;
};

struct MockStolenMemoryInHeap : public StolenMemoryInHeap {
    MockStolenMemoryInHeap(uint32_t deviceCount, uint64_t size) : StolenMemoryInHeap(deviceCount, size) {
    }
    using StolenMemoryInHeap::localStolenStorage;
};

} // namespace aub_stream
