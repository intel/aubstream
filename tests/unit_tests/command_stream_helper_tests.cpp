/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/memory_banks.h"

#include "aubstream/allocation_params.h"
#include "aubstream/engine_node.h"
#include "tests/unit_tests/command_stream_helper_tests.h"

#include "test.h"
#include "test_defaults.h"

#include <memory>
#include <vector>

using namespace aub_stream;

TEST_F(CommandStreamerHelperTest, WhenCommandStreamHelperIsInitializedThenLRCAIncludesLRISettingDebugMode) {
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);

    auto sizeLRCA = rcs.sizeLRCA;
    auto pLRCA = std::unique_ptr<uint32_t[]>(new uint32_t[rcs.sizeLRCA / sizeof(uint32_t)]);
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    rcs.initialize(reinterpret_cast<void *>(pLRCA.get()), &pageTable, 0);

    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x20d8, 0x00200020));
}

TEST_F(CommandStreamerHelperTest, WhenCommandStreamHelperIsInitializedThenLRCAIncludesBbCurrentHeadReg) {
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);

    auto sizeLRCA = rcs.sizeLRCA;
    auto pLRCA = std::unique_ptr<uint32_t[]>(new uint32_t[rcs.sizeLRCA / sizeof(uint32_t)]);
    PhysicalAddressAllocatorSimple allocator;
    PML4 pageTable(*gpu, &allocator, defaultMemoryBank);
    rcs.initialize(reinterpret_cast<void *>(pLRCA.get()), &pageTable, 0);

    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2168, 0));
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2140, 0));
}

TEST_F(CommandStreamerHelperTest, WhenCommandStreamHelperIsInitializedThenLRCAIncludesPDPRegisters) {
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);
    PhysicalAddressAllocatorSimple allocator;
    PDP4 pageTable(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    auto sizeLRCA = rcs.sizeLRCA;
    auto pLRCA = std::unique_ptr<uint32_t[]>(new uint32_t[rcs.sizeLRCA / sizeof(uint32_t)]);
    rcs.initialize(reinterpret_cast<void *>(pLRCA.get()), &pageTable, 0);

    auto physAddress = pageTable.getChild(0)->getPhysicalAddress();
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2270, uint32_t(physAddress)));
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2274, uint32_t(physAddress >> 32)));

    physAddress = pageTable.getChild(1)->getPhysicalAddress();
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2278, uint32_t(physAddress)));
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x227c, uint32_t(physAddress >> 32)));

    physAddress = pageTable.getChild(2)->getPhysicalAddress();
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2280, uint32_t(physAddress)));
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2284, uint32_t(physAddress >> 32)));

    physAddress = pageTable.getChild(3)->getPhysicalAddress();
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2288, uint32_t(physAddress)));
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x228c, uint32_t(physAddress >> 32)));
}

TEST_F(CommandStreamerHelperTest, WhenCommandStreamHelperIsInitializedThenLRCAIncludesPML4Register) {
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);
    PhysicalAddressAllocatorSimple allocator;
    PDP4 pageTable(*gpu, &allocator, MEMORY_BANK_SYSTEM);

    // Initialize page table to a known state
    uint32_t data = 0xbaddf00d;
    stream.writeMemory(&pageTable, {0x00000000, &data, sizeof(data), MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, defaultPageSize});

    auto sizeLRCA = rcs.sizeLRCA;
    auto pLRCA = std::unique_ptr<uint32_t[]>(new uint32_t[rcs.sizeLRCA / sizeof(uint32_t)]);
    rcs.initialize(reinterpret_cast<void *>(pLRCA.get()), &pageTable, 0);

    EXPECT_FALSE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2270, 0x00000000)); // Expecting false, we just want to make sure a 0 physical address is not used
    EXPECT_TRUE(checkLRIInLRCA(pLRCA.get(), sizeLRCA, rcs.mmioEngine, 0x2274, 0x00000000));
}

TEST_F(CommandStreamerHelperTest, csHelperHasValidParams) {
    const uint32_t numEngines = 4;
    EngineType engines[numEngines] = {ENGINE_RCS, ENGINE_BCS, ENGINE_VCS, ENGINE_VECS};

    for (int i = 0; i < numEngines; i++) {
        if (!gpu->isEngineSupported(engines[i])) {
            continue;
        }

        auto &cs = gpu->getCommandStreamerHelper(defaultDevice, engines[i]);

        if (EngineType::ENGINE_RCS == engines[i]) {
            EXPECT_EQ(DataTypeHintValues::TraceLogicalRingContextRcs, cs.aubHintLRCA);
            EXPECT_EQ(DataTypeHintValues::TraceCommandBufferPrimary, cs.aubHintCommandBuffer);
            EXPECT_EQ(DataTypeHintValues::TraceBatchBufferPrimary, cs.aubHintBatchBuffer);
            EXPECT_EQ(0x11000u, cs.sizeLRCA);
        } else if (EngineType::ENGINE_BCS == engines[i]) {
            EXPECT_EQ(DataTypeHintValues::TraceLogicalRingContextBcs, cs.aubHintLRCA);
            EXPECT_EQ(DataTypeHintValues::TraceCommandBufferBlt, cs.aubHintCommandBuffer);
            EXPECT_EQ(DataTypeHintValues::TraceBatchBufferBlt, cs.aubHintBatchBuffer);
            EXPECT_EQ(0x2000u, cs.sizeLRCA);
        } else if (EngineType::ENGINE_VCS == engines[i]) {
            EXPECT_EQ(DataTypeHintValues::TraceLogicalRingContextVcs, cs.aubHintLRCA);
            EXPECT_EQ(DataTypeHintValues::TraceCommandBufferMfx, cs.aubHintCommandBuffer);
            EXPECT_EQ(DataTypeHintValues::TraceBatchBufferMfx, cs.aubHintBatchBuffer);
            EXPECT_EQ(0x2000u, cs.sizeLRCA);
        } else if (EngineType::ENGINE_VECS == engines[i]) {
            EXPECT_EQ(DataTypeHintValues::TraceLogicalRingContextVecs, cs.aubHintLRCA);
            EXPECT_EQ(DataTypeHintValues::TraceCommandBuffer, cs.aubHintCommandBuffer);
            EXPECT_EQ(DataTypeHintValues::TraceBatchBuffer, cs.aubHintBatchBuffer);
            EXPECT_EQ(0x2000u, cs.sizeLRCA);
        } else {
            EXPECT_TRUE(false);
        }
    }
}

TEST_F(CommandStreamerHelperTest, CheckRCSFlushCommands) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);
    std::vector<uint32_t> testRingBuffer{};

    rcs.addFlushCommands(testRingBuffer);

    size_t sizeOfCommands = 0;
    // Verify the first Pipe Control
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x7a000004);
    // We are testing for:
    // CSStallEnable
    // RenderTargetCacheFlushEnable
    // DCFlushEnable
    // IndirectStatePointersDisable
    // DepthCacheFlushEnable
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x00101221);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Verify the second Pipe Control
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x7a000004);
    // We are testing for:
    // CSStallEnable
    // IndirectStatePointersDisable
    // StateCacheInvalidationEnable
    // ConstantCacheInvalidationEnable
    // VFCacheInvalidationEnable
    // DCFlushEnable
    // TextureCacheInvalidateEnable
    // InstructionCacheInvalidateEnable
    // CommandCacheInvalidateEnable
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x20100E3C);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

TEST_F(CommandStreamerHelperTest, CheckBCSFlushCommands) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_BCS));
    auto &bcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_BCS);
    std::vector<uint32_t> testRingBuffer{};

    bcs.addFlushCommands(testRingBuffer);

    size_t sizeOfCommands = 0;
    // Verify the flush dword
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x13000003);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

TEST_F(CommandStreamerHelperTest, CheckVCSFlushCommands) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_VCS));
    auto &vcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_VCS);
    std::vector<uint32_t> testRingBuffer{};

    vcs.addFlushCommands(testRingBuffer);

    size_t sizeOfCommands = 0;
    // Verify the flush dword
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x13000003);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

TEST_F(CommandStreamerHelperTest, CheckVECSFlushCommands) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_VECS));
    auto &vecs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_VECS);
    std::vector<uint32_t> testRingBuffer{};

    vecs.addFlushCommands(testRingBuffer);

    size_t sizeOfCommands = 0;
    // Verify the flush dword
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x13000003);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

TEST_F(CommandStreamerHelperTest, CheckCCSFlushCommands) {
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_CCS));
    TEST_REQUIRES(!gpu->getCommandStreamerHelper(defaultDevice, ENGINE_CCS).isExeclistSubmissionEnabled());

    auto &ccs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_CCS);
    std::vector<uint32_t> testRingBuffer{};

    ccs.addFlushCommands(testRingBuffer);

    size_t sizeOfCommands = 0;
    // Verify the first Pipe Control
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x7a000004);
    // We are testing for:
    // CSStallEnable
    // DCFlushEnable
    // IndirectStatePointersDisable
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x00100220);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Verify the second Pipe Control
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x7a000004);
    // We are testing for:
    // CSStallEnable
    // IndirectStatePointersDisable
    // StateCacheInvalidationEnable
    // ConstantCacheInvalidationEnable
    // DCFlushEnable
    // TextureCacheInvalidateEnable
    // InstructionCacheInvalidateEnable
    // CommandCacheInvalidateEnable
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x20100E2C);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

TEST_F(CommandStreamerHelperTest, CheckStoreFence) {
    // The function is the same for all engines so far
    TEST_REQUIRES(gpu->isEngineSupported(ENGINE_RCS));
    auto &rcs = gpu->getCommandStreamerHelper(defaultDevice, ENGINE_RCS);
    std::vector<uint32_t> testRingBuffer{};

    rcs.storeFenceValue(testRingBuffer, 0x0badc0fedeadbeefull, 0x8086);

    size_t sizeOfCommands = 0;
    // Verify the store DWORD
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x10400002);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0xdeadbeef);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x8086);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

static std::map<EngineType, uint32_t> engineBases{
    {ENGINE_RCS, 0x00000},
    {ENGINE_BCS, 0x20000},
    {ENGINE_VCS, 0x1be000},
    {ENGINE_VECS, 0x1c6000},
};

class CommandStreamerHelperVerifyEngineMmioTest : public ::testing::TestWithParam<std::tuple<uint32_t, std::pair<const EngineType, uint32_t>>> {
};

TEST_P(CommandStreamerHelperVerifyEngineMmioTest, initializeEngineMMIO) {
    auto device = std::get<0>(GetParam());
    auto engine = std::get<1>(GetParam()).first;
    TEST_REQUIRES(device < gpu->deviceCount);
    TEST_REQUIRES(gpu->isEngineSupported(engine));

    auto deviceBase = device * 16 * MB;
    auto csBase = std::get<1>(GetParam()).second + 0x2000;
    auto mmioBase = deviceBase + csBase;
    auto &cs = gpu->getCommandStreamerHelper(device, engine);
    VerifyMmioAubStream stream(mmioBase, mmioBase + 0x7ff);

    cs.initializeEngineMMIO(stream);
}

TEST_P(CommandStreamerHelperVerifyEngineMmioTest, submit) {
    auto device = std::get<0>(GetParam());
    auto engine = std::get<1>(GetParam()).first;
    TEST_REQUIRES(device < gpu->deviceCount);
    TEST_REQUIRES(gpu->isEngineSupported(engine));

    auto deviceBase = device * 16 * MB;
    auto csBase = std::get<1>(GetParam()).second + 0x2000;
    auto mmioBase = deviceBase + csBase;
    auto &cs = gpu->getCommandStreamerHelper(device, engine);
    VerifyMmioAubStream stream(mmioBase, mmioBase + 0x7ff);

    auto ggttFakeLRCA = 0x1000;
    cs.submit(stream, ggttFakeLRCA, true, 0, 0);
}

TEST_P(CommandStreamerHelperVerifyEngineMmioTest, pollForCompletion) {
    auto device = std::get<0>(GetParam());
    auto engine = std::get<1>(GetParam()).first;
    TEST_REQUIRES(device < gpu->deviceCount);
    TEST_REQUIRES(gpu->isEngineSupported(engine));

    auto deviceBase = device * 16 * MB;
    auto csBase = std::get<1>(GetParam()).second + 0x2000;
    auto mmioBase = deviceBase + csBase;
    auto &cs = gpu->getCommandStreamerHelper(device, engine);
    VerifyMmioAubStream stream(mmioBase, mmioBase + 0x7ff);

    cs.pollForCompletion(stream);
}

TEST_P(CommandStreamerHelperVerifyEngineMmioTest, CheckBatchBufferStart) {
    auto device = std::get<0>(GetParam());
    auto engine = std::get<1>(GetParam()).first;
    TEST_REQUIRES(device < gpu->deviceCount);
    TEST_REQUIRES(gpu->isEngineSupported(engine));
    TEST_REQUIRES(!gpu->getCommandStreamerHelper(device, engine).isRingDataEnabled());

    auto deviceBase = device * 16 * MB;
    auto csBase = std::get<1>(GetParam()).second + 0x2000;
    auto mmioBase = deviceBase + csBase;
    auto &cs = gpu->getCommandStreamerHelper(device, engine);

    std::vector<uint32_t> testRingBuffer{};
    const uint64_t bufferAddress = 0x0badc0fedeadbeefull;
    cs.addBatchBufferJump(testRingBuffer, bufferAddress);

    size_t sizeOfCommands = 0;
    // Verify the LRI
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x11000001);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], mmioBase + 0x244);
    // Inhibit synchronous context switch
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x00090008);
    // Verify the Batch Buffer start
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x18800101);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0xdeadbeef);
    EXPECT_EQ(testRingBuffer[sizeOfCommands++], 0x0badc0fe);

    // Finally make sure that we don't have any more commands
    // which the test doesn't know about
    EXPECT_EQ(sizeOfCommands, testRingBuffer.size());
}

INSTANTIATE_TEST_SUITE_P(VerifyMMIO,
                         CommandStreamerHelperVerifyEngineMmioTest,
                         ::testing::Combine(
                             ::testing::Range(0u, 4u),
                             ::testing::ValuesIn(engineBases)));
