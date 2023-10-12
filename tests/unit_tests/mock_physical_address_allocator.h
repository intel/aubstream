/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/physical_address_allocator.h"
#include "gmock/gmock.h"

namespace aub_stream {

template <typename AddressType>
struct MockSimpleAllocator : public SimpleAllocator<AddressType> {
    using BaseClass = SimpleAllocator<AddressType>;
    using SimpleAllocator<AddressType>::nextAddress;

    explicit MockSimpleAllocator(AddressType firstAddress)
        : BaseClass(firstAddress) {
    }

    virtual AddressType alignedAlloc(size_t size, AddressType alignment) override {
        alignedAllocCalled = true;
        return BaseClass::alignedAlloc(size, alignment);
    }

    virtual void alignedFree(uint64_t address) override {
        alignedFreeCalled = true;
        return BaseClass::alignedFree(address);
    }
    bool alignedAllocCalled = false;
    bool alignedFreeCalled = false;
};

struct MockPhysicalAddressAllocatorSimple : public PhysicalAddressAllocatorSimple {
    MockPhysicalAddressAllocatorSimple() : PhysicalAddressAllocatorSimple() {
    }

    MockPhysicalAddressAllocatorSimple(uint32_t numberOfAllocators, size_t singleAllocatorSize, bool localMemorySupport) : PhysicalAddressAllocatorSimple(numberOfAllocators, singleAllocatorSize, localMemorySupport) {
    }

    using PhysicalAddressAllocatorSimple::allocators;
    using PhysicalAddressAllocatorSimple::numberOfAllocators;
};

struct MockPhysicalAddressAllocatorHeap : public PhysicalAddressAllocatorHeap {
    MockPhysicalAddressAllocatorHeap() : PhysicalAddressAllocatorHeap() {
    }

    using PhysicalAddressAllocatorHeap::storage;
};

struct MockPhysicalAddressAllocatorSimpleAndSHM4Mapper : public PhysicalAddressAllocatorSimpleAndSHM4Mapper {
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport, SharedMemoryInfo *smInfo) : PhysicalAddressAllocatorSimpleAndSHM4Mapper(numberOfAllocators, singleAllocatorSize, localMemorySupport, smInfo) {
    }
    MOCK_METHOD3(reservePhysicalMemory, uint64_t(uint32_t memoryBank, size_t size, size_t alignment));
    MOCK_METHOD5(translatePhysicalAddressToSystemMemory, void(uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize));
    uint64_t reservePhysicalMemoryBase(uint32_t memoryBank, size_t size, size_t alignment) { return PhysicalAddressAllocatorSimpleAndSHM4Mapper::reservePhysicalMemory(memoryBank, size, alignment); };
    void translatePhysicalAddressToSystemMemoryBase(uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) { PhysicalAddressAllocatorSimpleAndSHM4Mapper::translatePhysicalAddressToSystemMemory(physAddress, size, isLocalMemory, p, availableSize); };
    using PhysicalAddressAllocatorSimple::allocators;
    using PhysicalAddressAllocatorSimple::numberOfAllocators;
    using PhysicalAddressAllocatorSimpleAndSHM4Mapper::storage;
};

} // namespace aub_stream
