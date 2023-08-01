/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "alloc_tools.h"
#include "aubstream/shared_mem_info.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <list>
#include <vector>

namespace aub_stream {
const size_t reservedGGTTSpace = 0x80000;

template <typename AddressType>
struct SimpleAllocator {
    explicit SimpleAllocator(AddressType firstAddress)
        : nextAddress(firstAddress) {
    }

    AddressType alignedAlloc(size_t size, AddressType alignment) {
        alignment = std::max(alignment, AddressType(4096u));

        std::lock_guard<std::mutex> guard(mutex);
        nextAddress += alignment - 1;
        nextAddress &= ~(alignment - 1);
        auto physicalAddress = nextAddress;
        nextAddress += AddressType(size);
        return physicalAddress;
    }

  protected:
    std::mutex mutex;
    AddressType nextAddress;
};

struct PhysicalAddressAllocator {
    virtual ~PhysicalAddressAllocator() = default;
    virtual uint64_t reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) = 0;

    static constexpr uint32_t mainBank = 0;
};

struct PhysicalAddressAllocatorSimple : public PhysicalAddressAllocator {
    PhysicalAddressAllocatorSimple() : mainAllocator(reservedGGTTSpace) {
    }

    PhysicalAddressAllocatorSimple(uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport);
    uint64_t reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) override;

  protected:
    SimpleAllocator<uint64_t> mainAllocator;
    uint32_t numberOfAllocators = 0;
    uint64_t allocatorSize = 0x80000000;
    std::vector<std::unique_ptr<SimpleAllocator<uint64_t>>> allocators;
};

struct PhysicalAddressAllocatorHeap : public PhysicalAddressAllocator {
    PhysicalAddressAllocatorHeap() {
    }
    uint64_t reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) override;

  protected:
    std::list<std::unique_ptr<uint8_t, decltype(&aligned_free)>> storage;
};

struct PhysicalAddressAllocatorSimpleAndSHM4Mapper : public PhysicalAddressAllocatorSimple {
    PhysicalAddressAllocatorSimpleAndSHM4Mapper(uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport, SharedMemoryInfo *smInfo);
    uint64_t reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment) override;

    virtual uint64_t reserveOnlyPhysicalSpace(uint32_t memoryBank, size_t size, size_t alignment);
    virtual void mapSystemMemoryToPhysicalAddress(uint64_t physAddress, size_t size, size_t alignment, bool isLocalMemory, const void *p);
    virtual void translatePhysicalAddressToSystemMemory(uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize);

  protected:
    typedef uint8_t *TranslationTableElement;
    typedef const uint8_t *ConstTranslationTableElement;

    SharedMemoryInfo *sharedMemoryInfo = nullptr;
    std::list<std::unique_ptr<uint8_t, decltype(&aligned_free)>> storage;
};

} // namespace aub_stream
