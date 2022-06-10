/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "physical_address_allocator.h"
#include <algorithm>
#include <cassert>

namespace aub_stream {

uint64_t PhysicalAddressAllocator::reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) {
    if (memoryBank == 0 || numberOfAllocators == 0) {
        return mainAllocator.alignedAlloc(size, alignment);
    }

    uint32_t allocatorIndex = 0;
    while ((memoryBank & 1u) == 0) {
        memoryBank >>= 1;
        allocatorIndex++;
    }

    assert(allocatorIndex < allocators.size());

    return allocators[allocatorIndex]->alignedAlloc(size, alignment);
}

} // namespace aub_stream
