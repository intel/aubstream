/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/aub_stream.h"
#include "aubstream/aubstream.h"
#include "aubstream/hardware_context.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "gtest/gtest.h"

namespace aub_stream {
struct SimpleBatchBuffer : public ::testing::TestWithParam<uint32_t> {
    SimpleBatchBuffer()
        : desc(*gpu),
          mgr(nullptr),
          ctxt(nullptr) {
    }

    void SetUp() override {
        // Default to the minimal sku
        desc.deviceCount = 1;
    }

    void TearDown() override {
        delete ctxt;
        delete mgr;
    }

    void initializeStream(const GpuDescriptor &desc) {
        auto gpu = createGpuFunc();
        auto fileName = getAubFileName(desc);
        assert(!mgr);
        auto supportsLocalMemory =
            gpu->isMemorySupported(MEMORY_BANK_0, 4096u) ||
            gpu->isMemorySupported(MEMORY_BANK_0, 65536u);
        AubManagerOptions internal_options;
        internal_options.devicesCount = gpu->deviceCount;
        internal_options.memoryBankSize = defaultHBMSizePerDevice;
        internal_options.stepping = SteppingValues::A;
        internal_options.localMemorySupported = supportsLocalMemory;
        internal_options.mode = streamMode;
        internal_options.gpuAddressSpace = gpuAddressSpace48;
        mgr = new AubManagerImp(std::move(gpu), internal_options);
        mgr->open(fileName);
    }

    GpuDescriptor desc;
    AubManagerImp *mgr;
    HardwareContext *ctxt;
    const uint32_t streamMode = mode::aubFile;
};
} // namespace aub_stream
