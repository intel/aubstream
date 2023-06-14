/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "physical_address_allocator.h"
#include <cassert>

namespace aub_stream {

PhysicalAddressAllocatorSimple::PhysicalAddressAllocatorSimple(uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport) : mainAllocator(reservedGGTTSpace),
                                                                                                                                                     numberOfAllocators(numberOfAllocators),
                                                                                                                                                     allocatorSize(singleAllocatorSize) {
    allocators.resize(numberOfAllocators);

    uint64_t startAddress = localMemorySupport ? allocatorSize / 256 : 0;
    for (auto &allocator : allocators) {
        allocator = std::make_unique<SimpleAllocator<uint64_t>>(std::max(startAddress, uint64_t(0x1000u)));
        startAddress += allocatorSize;
    }
}

uint64_t PhysicalAddressAllocatorSimple::reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) {
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

uint64_t PhysicalAddressAllocatorHeap::reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) {
    auto p = std::unique_ptr<uint8_t, decltype(&aligned_free)>(reinterpret_cast<uint8_t *>(aligned_alloc(size, alignment)), &aligned_free);
    assert(p);
    static_assert(sizeof(uint8_t *) <= sizeof(uint64_t), "Pointer type must be less or equal 64b");
    uint64_t res = reinterpret_cast<uint64_t>(p.get());
    storage.push_back(std::move(p));
    return res;
}

std::unique_ptr<PhysicalAddressAllocator> PhysicalAddressAllocator::CreatePhysicalAddressAllocator(bool inHeap, uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport) {
    if (inHeap) {
        return std::unique_ptr<PhysicalAddressAllocator>(new PhysicalAddressAllocatorHeap);
    } else {
        return std::unique_ptr<PhysicalAddressAllocator>(new PhysicalAddressAllocatorSimple(numberOfAllocators, singleAllocatorSize, localMemorySupport));
    }
}

} // namespace aub_stream
