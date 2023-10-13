/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/page_table.h"
#include "gtest/gtest.h"

#include <cstdint>

struct PageTableHelper {
    static inline uint64_t getEntry(aub_stream::PageTable *ppgtt, uint64_t gfxAddress) {
        aub_stream::PageTable *pageTable = ppgtt->getChild(ppgtt->getIndex(gfxAddress));

        auto levels = ppgtt->getNumLevels() - 1;
        while (levels-- > 0) {
            EXPECT_NE(nullptr, pageTable);
            pageTable = pageTable->getChild(pageTable->getIndex(gfxAddress));
        }

        if (pageTable) {
            return pageTable->getPhysicalAddress();
        }
        return 0;
    }

    static inline uint64_t getPTEEntry(aub_stream::PageTable *ppgtt, uint64_t gfxAddress) {
        aub_stream::PageTable *pageTable = ppgtt;

        auto levels = ppgtt->getNumLevels() - 1;
        while (levels > 0) {
            EXPECT_NE(nullptr, pageTable);
            pageTable = pageTable->getChild(pageTable->getIndex(gfxAddress));
            levels--;
        }

        if (pageTable) {
            return pageTable->getEntryValue();
        }
        return 0;
    }
};
