/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
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
    PhysicalAddressAllocator() : mainAllocator(reservedGGTTSpace) {
    }

    PhysicalAddressAllocator(uint32_t numberOfAllocators, uint64_t singleAllocatorSize, bool localMemorySupport) : mainAllocator(reservedGGTTSpace),
                                                                                                                   numberOfAllocators(numberOfAllocators),
                                                                                                                   allocatorSize(singleAllocatorSize) {
        allocators.resize(numberOfAllocators);

        uint64_t startAddress = localMemorySupport ? allocatorSize / 256 : 0;
        for (auto &allocator : allocators) {
            allocator = std::make_unique<SimpleAllocator<uint64_t>>(std::max(startAddress, uint64_t(0x1000u)));
            startAddress += allocatorSize;
        }
    }
    uint64_t reservePhysicalMemory(uint32_t memoryBank, size_t size, size_t alignment);

    static constexpr uint32_t mainBank = 0;

  protected:
    SimpleAllocator<uint64_t> mainAllocator;
    uint32_t numberOfAllocators = 0;
    uint64_t allocatorSize = 0x80000000;
    std::vector<std::unique_ptr<SimpleAllocator<uint64_t>>> allocators;
};

} // namespace aub_stream
