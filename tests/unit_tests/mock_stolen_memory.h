/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/gpu.h"

namespace aub_stream {

struct MockStolenMemoryInStaticStorage : public StolenMemoryInStaticStorage {
    MockStolenMemoryInStaticStorage(uint64_t memoryBankSize) : StolenMemoryInStaticStorage(memoryBankSize) {
    }
    using StolenMemoryInStaticStorage::staticMemoryBankSize;
};

struct MockStolenMemoryInHeap : public StolenMemoryInHeap {
    MockStolenMemoryInHeap(uint32_t deviceCount, uint64_t memoryBankSize) : StolenMemoryInHeap(deviceCount, memoryBankSize) {
    }
    using StolenMemoryInHeap::localStolenStorage;
};

} // namespace aub_stream
