/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aub_mem_dump/page_table.h"
#include "aubstream/headers/page_info.h"
#include "aubstream/headers/allocation_params.h"

namespace aub_stream {

struct PageEntryInfo {
    uint64_t physicalAddress;
    uint64_t tableEntry;
};

class PageTableWalker {
  public:
    enum class WalkMode {
        Reserve,
        Clone,
        Expect
    };

    std::vector<PageInfo> entries;
    std::vector<PageEntryInfo> pageWalkEntries[5];
    std::vector<uint64_t> pages64KB;

    void walkMemory(GGTT *pageTable, uint64_t gfxAddress, size_t size, uint32_t memoryBanks, size_t pageSize, WalkMode mode, const std::vector<PageInfo> *pageInfos);
    void walkMemory(PageTable *pageTable, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos);
    void walkMemory(PageTable *pageTable, const AllocationParams &allocationParams, WalkMode mode, const std::vector<PageInfo> *pageInfos, uint64_t physicalAddress);
};

} // namespace aub_stream
