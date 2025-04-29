/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/aub_stream.h"
#include "aubstream/aubstream.h"
#include "aubstream/hardware_context.h"
#include "aub_mem_dump/settings.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "gtest/gtest.h"
#include "aub_mem_dump/options.h"
#include "variable_backup.h"

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
        gpu->deviceCount = desc.deviceCount;
        gpu->deviceId = desc.deviceId;

        assert(gpu->gfxCoreFamily == desc.gfxCoreFamily);
        assert(gpu->productAbbreviation == desc.productAbbreviation);
        assert(gpu->productFamily == desc.productFamily);

        auto fileName = getAubFileName(*gpu);
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
        mgr->initialize();
        mgr->open(fileName);
    }

    void initializeStreamWithMultipleCCS(const GpuDescriptor &desc) {
        VariableBackup<MMIOList> injectListRestorer(&MMIOListInjected);
        injectMMIOList(MMIOList{MMIOPair(0x00014804, 0xFFF0688)});
        initializeStream(desc);
    }

    GpuDescriptor desc;
    AubManagerImp *mgr;
    HardwareContext *ctxt;
    const uint32_t streamMode = mode::aubFile;
};
} // namespace aub_stream
