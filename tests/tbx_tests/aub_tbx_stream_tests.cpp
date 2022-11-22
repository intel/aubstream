/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gtest/gtest.h"
#include "test_defaults.h"
#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aubstream/hardware_context.h"
#include "aubstream/aubstream.h"
#include "tests/simple_batch_buffer.h"

using namespace aub_stream;
struct AubTbxSimpleBatchBuffer : public ::testing::Test {
    AubTbxSimpleBatchBuffer()
        : desc(*gpu),
          mgr(nullptr),
          ctxt(nullptr) {
    }

    void SetUp() override;
    void TearDown() override;

    void initializeStream(const GpuDescriptor &desc);

    GpuDescriptor desc;
    AubManagerImp *mgr;
    HardwareContext *ctxt;
};

void AubTbxSimpleBatchBuffer::initializeStream(const GpuDescriptor &desc) {
    assert(!mgr);
    AubManagerOptions internal_options;
    internal_options.devicesCount = gpu->deviceCount;
    internal_options.memoryBankSize = defaultHBMSizePerDevice;
    internal_options.stepping = 0u;
    internal_options.localMemorySupported = true;
    internal_options.mode = mode::aubFileAndTbx;
    internal_options.gpuAddressSpace = gpuAddressSpace48;
    mgr = new AubManagerImp(*gpu, internal_options);
}

void AubTbxSimpleBatchBuffer::SetUp() {
    desc.deviceCount = 1;
}

void AubTbxSimpleBatchBuffer::TearDown() {
    delete ctxt;
    delete mgr;
}

TEST_F(AubTbxSimpleBatchBuffer, simpleBatchBufferRCS) {
    initializeStream(desc);

    ctxt = mgr->createHardwareContext(defaultDevice, ENGINE_RCS, 0);
    addSimpleBatchBuffer(ctxt, defaultMemoryBank);
    ctxt->pollForCompletion();
}
