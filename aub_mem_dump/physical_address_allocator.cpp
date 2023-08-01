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

void PhysicalAddressAllocatorSimpleAndSHM4Mapper::translatePhysicalAddressToSystemMemory(uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
    const uint64_t entryNumber = (physAddress / uint64_t(0x1000));
    const size_t pageOffset = static_cast<size_t>(physAddress % uint64_t(0x1000));
    TranslationTableElement *traslationTable = reinterpret_cast<TranslationTableElement *>(isLocalMemory ? sharedMemoryInfo->localMemBase
                                                                                                         : sharedMemoryInfo->sysMemBase);
    traslationTable += entryNumber;
    availableSize = 0;
    p = nullptr;
    if (*traslationTable != nullptr) {
        p = *traslationTable + pageOffset;
        availableSize = size_t(0x1000) - pageOffset;
        // Memory continuity check (if the entire area is available without
        // additional translations on page boundaries)
        uint8_t *expected = *traslationTable++ + 0x1000;
        size -= std::min(availableSize, size);
        while (size > 0) {
            if (*traslationTable != expected) {
                break;
            }
            availableSize += std::min<size_t>(0x1000, size);
            size -= std::min<size_t>(0x1000, size);
            expected += 0x1000;
            traslationTable++;
        }
    }
}

PhysicalAddressAllocatorSimpleAndSHM4Mapper::PhysicalAddressAllocatorSimpleAndSHM4Mapper(uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport, SharedMemoryInfo *smInfo) : PhysicalAddressAllocatorSimple(numberOfAllocators, singleAllocatorSize, localMemorySupport),
                                                                                                                                                                                                         sharedMemoryInfo(smInfo) {
}

uint64_t PhysicalAddressAllocatorSimpleAndSHM4Mapper::reserveOnlyPhysicalSpace(uint32_t memoryBank, size_t size, size_t alignment) {
    return PhysicalAddressAllocatorSimple::reservePhysicalMemory(memoryBank, size, alignment);
}

uint64_t PhysicalAddressAllocatorSimpleAndSHM4Mapper::reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) {
    uint64_t physAddress = reserveOnlyPhysicalSpace(memoryBank, size, alignment);
    mapSystemMemoryToPhysicalAddress(physAddress, size, alignment, memoryBank != 0 && numberOfAllocators != 0, nullptr);
    return physAddress;
}

void PhysicalAddressAllocatorSimpleAndSHM4Mapper::mapSystemMemoryToPhysicalAddress(uint64_t physAddress, size_t size, size_t alignment, bool isLocalMemory, const void *p) {
    if (p == nullptr) {
        auto up = std::unique_ptr<uint8_t, decltype(&aligned_free)>(reinterpret_cast<uint8_t *>(aligned_alloc(size, std::max<size_t>(alignment, 0x1000))), &aligned_free);
        assert(up);
        p = up.get();
        storage.push_back(std::move(up));
    }
    assert(physAddress % uint64_t(0x1000) == 0);
    assert(size % size_t(0x1000) == 0);
    uint64_t entryNumber = (physAddress / uint64_t(0x1000));
    ConstTranslationTableElement *traslationTable = reinterpret_cast<ConstTranslationTableElement *>(isLocalMemory ? sharedMemoryInfo->localMemBase
                                                                                                                   : sharedMemoryInfo->sysMemBase);
    traslationTable += entryNumber;
    while (size > 0) {
        *traslationTable++ = static_cast<ConstTranslationTableElement>(p);
        size -= 0x1000;
        p = reinterpret_cast<const uint8_t *>(p) + 0x1000;
    }
}

} // namespace aub_stream
