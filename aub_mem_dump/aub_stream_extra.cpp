/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/page_table_walker.h"

namespace aub_stream {

void AubStream::writePageWalkEntries(const PageTableWalker &pageWalker, bool pageTablesInLocalMemory, uint32_t numAddressBits) {
    if (pageTablesInLocalMemory) {
        writeDiscontiguousPages(pageWalker.pageWalkEntries[3], AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel4);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[2], AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel3);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[1], AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel2);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[0], AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1);
    } else {
        writeDiscontiguousPages(pageWalker.pageWalkEntries[3], AddressSpaceValues::TracePml4Entry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[2], AddressSpaceValues::TracePhysicalPdpEntry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[1], AddressSpaceValues::TracePpgttPdEntry, DataTypeHintValues::TraceNotype);
        writeDiscontiguousPages(pageWalker.pageWalkEntries[0], AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype);
    }
}

void AubStream::writePpgttLevel1(const std::vector<PageEntryInfo> &pageWalkEntry, bool pageTablesInLocalMemory, uint32_t numAddressBits) {
    if (pageTablesInLocalMemory) {
        writeDiscontiguousPages(pageWalkEntry, AddressSpaceValues::TraceLocal, DataTypeHintValues::TracePpgttLevel1);
    } else {
        writeDiscontiguousPages(pageWalkEntry, AddressSpaceValues::TracePpgttEntry, DataTypeHintValues::TraceNotype);
    }
}

} // namespace aub_stream
