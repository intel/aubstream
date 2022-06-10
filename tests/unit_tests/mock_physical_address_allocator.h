/*
 * Copyright (C) 2022 Intel Corporation
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

struct MockPhysicalAddressAllocator : public PhysicalAddressAllocator {
    MockPhysicalAddressAllocator() : PhysicalAddressAllocator() {
    }

    MockPhysicalAddressAllocator(uint32_t numberOfAllocators, size_t singleAllocatorSize, bool localMemorySupport) : PhysicalAddressAllocator(numberOfAllocators, singleAllocatorSize, localMemorySupport) {
    }

    using PhysicalAddressAllocator::allocators;
    using PhysicalAddressAllocator::numberOfAllocators;
    using PhysicalAddressAllocator::PhysicalAddressAllocator;
};
} // namespace aub_stream
