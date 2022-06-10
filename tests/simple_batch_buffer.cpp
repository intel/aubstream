/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "simple_batch_buffer.h"

namespace aub_stream {

void addSimpleBatchBuffer(HardwareContext *context, uint32_t memoryBank) {
    auto ctxt = static_cast<HardwareContextImp *>(context);
    ctxt->initialize();

    // Initialize batch buffer
    uint32_t batchCommands[] = {
        0x00000001,
        0x00000002,
        0x00000003,
        0x00000004,
        0x05000000,
    };

    // Allocate an arbitrary address from the GGTT allocator.
    // Could be any valid GPU address
    auto &allocator = ctxt->ggtt.gfxAddressAllocator;
    uintptr_t ppgttBatchBuffer = allocator.alignedAlloc(0x1000, 0x1000);

    ctxt->writeAndSubmitBatchBuffer(ppgttBatchBuffer, batchCommands, sizeof(batchCommands), memoryBank, defaultPageSize);
}
} // namespace aub_stream
