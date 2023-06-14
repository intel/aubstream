/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/physical_address_allocator.h"

namespace aub_stream {

template <typename AddressType>
struct MockSimpleAllocator : public SimpleAllocator<AddressType> {
    using SimpleAllocator<AddressType>::nextAddress;
};

struct MockPhysicalAddressAllocatorSimple : public PhysicalAddressAllocatorSimple {
    MockPhysicalAddressAllocatorSimple() : PhysicalAddressAllocatorSimple() {
    }

    MockPhysicalAddressAllocatorSimple(uint32_t numberOfAllocators, size_t singleAllocatorSize, bool localMemorySupport) : PhysicalAddressAllocatorSimple(numberOfAllocators, singleAllocatorSize, localMemorySupport) {
    }

    using PhysicalAddressAllocatorSimple::allocators;
    using PhysicalAddressAllocatorSimple::numberOfAllocators;
    using PhysicalAddressAllocatorSimple::PhysicalAddressAllocatorSimple;
};

struct MockPhysicalAddressAllocatorHeap : public PhysicalAddressAllocatorHeap {
    MockPhysicalAddressAllocatorHeap() : PhysicalAddressAllocatorHeap() {
    }

    using PhysicalAddressAllocatorHeap::storage;
};
} // namespace aub_stream
