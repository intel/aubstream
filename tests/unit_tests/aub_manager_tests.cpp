/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_file_stream.h"
#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/tbx_stream.h"
#include "aubstream/aubstream.h"
#include "aubstream/hardware_context.h"

#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "tests/unit_tests/mock_gpu.h"
#include "tests/unit_tests/page_table_helper.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;

TEST(AubManagerTest, givenNotSupportedGenWhenAubManagerIsCreatedThenNullptrIsReturned) {
    auto aubManager = AubManager::create(ProductFamily::MaxProduct, defaultDeviceCount, defaultHBMSizePerDevice, defaultStepping, true, mode::aubFile, maxNBitValue(48));

    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenNotSupportedGenWhenAubManagerIsCreatedUsingOldInterfaceThenNullptrIsReturnedUsingOptions) {
    AubManagerOptions internal_options;
    internal_options.version = 0;
    internal_options.productFamily = 0;
    internal_options.devicesCount = defaultDeviceCount;
    internal_options.memoryBankSize = defaultHBMSizePerDevice;
    internal_options.stepping = defaultStepping;
    internal_options.localMemorySupported = true;
    internal_options.mode = mode::aubFile;
    internal_options.gpuAddressSpace = maxNBitValue(48);
    auto aubManager = AubManager::create(internal_options);

    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenNotSupportedGenWhenAubManagerIsCreatedThenNullptrIsReturnedUsingOptions) {
    AubManagerOptions internal_options;
    internal_options.version = 1;
    internal_options.productFamily = static_cast<uint32_t>(ProductFamily::MaxProduct);
    internal_options.devicesCount = defaultDeviceCount;
    internal_options.memoryBankSize = defaultHBMSizePerDevice;
    internal_options.stepping = defaultStepping;
    internal_options.localMemorySupported = true;
    internal_options.mode = mode::aubFile;
    internal_options.gpuAddressSpace = maxNBitValue(48);
    auto aubManager = AubManager::create(internal_options);

    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenSupportedProductFamilyWhenAubManagerIsCreatedThenValidPtrIsReturned) {

    AubManagerOptions internal_options;
    internal_options.version = 1;
    internal_options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    internal_options.devicesCount = defaultDeviceCount;
    internal_options.memoryBankSize = defaultHBMSizePerDevice;
    internal_options.stepping = defaultStepping;
    internal_options.localMemorySupported = true;
    internal_options.mode = mode::aubFile;
    internal_options.gpuAddressSpace = maxNBitValue(48);
    auto aubManager = AubManager::create(gpu->productFamily, defaultDeviceCount, defaultHBMSizePerDevice, defaultStepping, true, mode::aubFile, maxNBitValue(48));

    EXPECT_NE(nullptr, aubManager);
    delete aubManager;
}

TEST(AubManagerImp, givenInvalidStreamModeWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse) {
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, std::numeric_limits<uint32_t>::max());
    EXPECT_FALSE(aubManager.isInitialized());

    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamTbxShm.get());
}

TEST(AubManagerImp, givenInvalidStreamModeWhenAubManagerCreateCalledThenNullptrReturned) {
    auto aubManager = AubManager::create(gpu->productFamily, 1, 32 * defaultPageSize, defaultStepping, true, std::numeric_limits<uint32_t>::max(), maxNBitValue(48));
    EXPECT_EQ(nullptr, aubManager);

    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = 32 * defaultPageSize;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = std::numeric_limits<uint32_t>::max();
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;

    aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerImp, givenInvalidStreamModeAndExceptionsEnabledWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = std::numeric_limits<uint32_t>::max();
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = true;

    EXPECT_THROW(AubManager::create(options), std::runtime_error);
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithAubFileModeAndOpenIsCalledThenItInitializesAubFileStream) {
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    EXPECT_NE(nullptr, aubManager.streamAub.get());

    aubManager.open("test.aub");
    EXPECT_TRUE(static_cast<AubFileStream *>(aubManager.streamAub.get())->isOpen());
    EXPECT_STREQ("test.aub", static_cast<AubFileStream *>(aubManager.streamAub.get())->getFileName().c_str());

    EXPECT_TRUE(aubManager.isOpen());
    EXPECT_STREQ("test.aub", aubManager.getFileName().c_str());

    aubManager.close();
    EXPECT_FALSE(static_cast<AubFileStream *>(aubManager.streamAub.get())->isOpen());
    EXPECT_TRUE(static_cast<AubFileStream *>(aubManager.streamAub.get())->getFileName().empty());

    EXPECT_FALSE(aubManager.isOpen());
    EXPECT_TRUE(aubManager.getFileName().empty());
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithTbxModeThenItInitializesTbxStream) {
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::tbx);

    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_TRUE(static_cast<TbxStream *>(aubManager.streamTbx.get())->socket);

    EXPECT_FALSE(aubManager.isOpen());
    EXPECT_TRUE(aubManager.getFileName().empty());
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithAubFileAndTbxModeThenItInitializesAubAndTbxStreams) {
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFileAndTbx);
    aubManager.open("test.aub");

    EXPECT_NE(nullptr, aubManager.streamAub.get());
    EXPECT_TRUE(static_cast<AubFileStream *>(aubManager.streamAub.get())->isOpen());
    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_TRUE(static_cast<TbxStream *>(aubManager.streamTbx.get())->socket);

    EXPECT_TRUE(aubManager.isOpen());
    EXPECT_STREQ("test.aub", aubManager.getFileName().c_str());
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithAubFileModeAndSteppingParamThenSteppingIsSetCorrect) {
    uint32_t stepping = 1u;
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, stepping, true, mode::aubFile);
    EXPECT_NE(nullptr, aubManager.streamAub.get());

    EXPECT_EQ(stepping, aubManager.stepping);
}

using AubManagerTest = ::testing::Test;

HWTEST_F(AubManagerTest, when2DevicesAreCreatedThenAubManagerIsInitializedCorrectly, MatchMultiDevice::moreThanOne) {

    bool localMemorySupport = false;
    MockAubManager aubManager(*gpu, 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    EXPECT_EQ(2u, aubManager.ppgtts.size());
    EXPECT_EQ(2u, aubManager.ggtts.size());

    if (!gpu->requireLocalMemoryForPageTables()) {
        EXPECT_EQ(MemoryBank::MEMORY_BANK_SYSTEM, aubManager.ppgtts[0].get()->getMemoryBank());
        EXPECT_EQ(MemoryBank::MEMORY_BANK_SYSTEM, aubManager.ppgtts[1].get()->getMemoryBank());
        EXPECT_EQ(MemoryBank::MEMORY_BANK_SYSTEM, aubManager.ggtts[0].get()->getMemoryBank());
        EXPECT_EQ(MemoryBank::MEMORY_BANK_SYSTEM, aubManager.ggtts[1].get()->getMemoryBank());
    }

    EXPECT_NE(nullptr, aubManager.physicalAddressAllocator.get());
    EXPECT_NE(nullptr, aubManager.streamAub.get());
}

HWTEST_F(AubManagerTest, when2DevicesWithLocalMemoryAreCreatedThenAubManagerIsInitializedCorrectly, MatchMultiDevice::moreThanOne) {
    bool localMemorySupport = true;
    MockAubManager aubManager(*gpu, 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    EXPECT_EQ(2u, aubManager.ppgtts.size());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_0, aubManager.ppgtts[0].get()->getMemoryBank());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_1, aubManager.ppgtts[1].get()->getMemoryBank());

    EXPECT_EQ(2u, aubManager.ggtts.size());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_0, aubManager.ggtts[0].get()->getMemoryBank());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_1, aubManager.ggtts[1].get()->getMemoryBank());

    EXPECT_NE(nullptr, aubManager.physicalAddressAllocator.get());
    EXPECT_NE(nullptr, aubManager.streamAub.get());
}

TEST(AubManagerImp, createHardwareContextShouldValidHardwareContext) {
    bool localMemorySupport = true;
    MockAubManager aubManager(*gpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    auto hardwareContext = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

    EXPECT_NE(nullptr, hardwareContext);
    delete hardwareContext;
}

HWTEST_F(AubManagerTest, whenAubManagerWritesMemoryThenPageTablesCloned, MatchMultiDevice::moreThanOne) {

    bool localMemorySupport = true;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    MockAubManager aubManager(*gpu, 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    aubManager.writeMemory2({gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, defaultPageSize});

    auto physicalAddress1 = PageTableHelper::getEntry(aubManager.ppgtts[0].get(), gfxAddress);
    auto physicalAddress2 = PageTableHelper::getEntry(aubManager.ppgtts[1].get(), gfxAddress);
    EXPECT_EQ(physicalAddress1, physicalAddress2);
}

HWTEST_F(AubManagerTest, whenAubManagerWritesMemoryThenPageTablesParamsCloned, MatchMultiDevice::moreThanOne) {
    bool localMemorySupport = true;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    MockAubManager aubManager(*gpu, 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    AllocationParams params(gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, defaultPageSize);
    params.additionalParams.compressionEnabled = true;

    aubManager.writeMemory2(params);

    for (auto &ppgtt : aubManager.ppgtts) {
        auto entry = ppgtt.get();

        for (int32_t level = static_cast<int32_t>(ppgtt->getNumLevels() - 1); level >= 0; level--) {
            entry = entry->getChild(0);

            if (level == 0) {
                EXPECT_TRUE(entry->peekAllocationParams().compressionEnabled);
                EXPECT_FALSE(entry->peekAllocationParams().uncached);
            } else {
                EXPECT_FALSE(entry->peekAllocationParams().compressionEnabled);
                EXPECT_FALSE(entry->peekAllocationParams().uncached);
            }
        }
    }
}

TEST(AubManager, whenAubManagerWritesPageTableEntiesThenPhysicalMemory) {
    bool localMemorySupport = false;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0x1000;
    std::vector<PageInfo> lastLevelentries;

    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    aubManager.writePageTableEntries(gfxAddress, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, lastLevelentries, defaultPageSize);

    EXPECT_EQ(lastLevelentries.size(), 1);
}

TEST(AubManager, initializeAlsoInitializesGlobalMmio) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(mockGpu, initializeGlobalMMIO(_, mockGpu.deviceCount, defaultHBMSizePerDevice, 0u));

    MockAubManager aubManager(mockGpu, mockGpu.deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.open("test.aub");
}

TEST(AubManager, initializeAlsoSetsMemoryBankSize) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(mockGpu, setMemoryBankSize(_, gpu->deviceCount, defaultHBMSizePerDevice));

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.open("test.aub");
}

TEST(AubManager, initializeAlsoSetsGGTTBaseAddresses) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(mockGpu, setGGTTBaseAddresses(_, gpu->deviceCount, defaultHBMSizePerDevice));

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::tbx);
    aubManager.open("test.aub");
}

TEST(AubManager, initializeAllocatesPageTablesFromGpu) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(mockGpu, allocatePPGTT(_, _, _)).Times(gpu->deviceCount);
    EXPECT_CALL(mockGpu, allocateGGTT(_, _, _)).Times(gpu->deviceCount);

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
}

TEST(AubManager, givenAubManagerCreatedWithAubFileModeWhenGetStreamIsCalledThenAubFileStreamIsReturned) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    EXPECT_EQ(aubManager.streamAub.get(), aubManager.getStream());
}

TEST(AubManager, givenAubManagerCreatedWithTbxModeWhenGetStreamIsCalledThenTbxStreamIsReturned) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::tbx);

    EXPECT_EQ(aubManager.streamTbx.get(), aubManager.getStream());
}

TEST(AubManager, givenAubManagerCreatedWithAubFileAndTbxModeWhenGetStreamIsCalledThenAubTbxStreamIsReturned) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFileAndTbx);

    EXPECT_EQ(aubManager.streamAubTbx.get(), aubManager.getStream());
}

TEST(AubManager, givenAubFileModeWhenCreatingAubManagerThenOnlyAubFileStreamIsCreated) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);

    EXPECT_NE(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
}

TEST(AubManager, givenTbxModeWhenCreatingAubManagerThenOnlyTbxStreamIsCreated) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::tbx);

    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
}

TEST(AubManager, givenAubFileAndTbxModeWhenCreatingAubManagerThenAllStreamsAreCreated) {
    MockGpu mockGpu;
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(mockGpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFileAndTbx);

    EXPECT_NE(nullptr, aubManager.streamAub.get());
    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_NE(nullptr, aubManager.streamAubTbx.get());
}

TEST(AubManager, givenAubManagerCreatedWithAubFileAndTbxModeWhenHardwareContextIsCreatedThenAubTbxStreamIsUsed) {
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(*gpu, gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFileAndTbx);
    aubManager.open("test.aub");

    struct MockAubTbxStream : public AubTbxStream {
        using AubTbxStream::AubTbxStream;
        MOCK_METHOD2(declareContextForDumping, void(uint32_t handleDumpContext, PageTable *pageTable));
    };

    auto aubFilestream = aubManager.streamAub.get();
    auto tbxStream = aubManager.streamTbx.get();

    auto mockAubTbxStream = new ::testing::NiceMock<MockAubTbxStream>(*aubFilestream, *tbxStream);

    aubManager.streamAubTbx.reset(mockAubTbxStream);
    EXPECT_CALL(*mockAubTbxStream, declareContextForDumping(_, _)).Times(1);
    auto hwContext = std::unique_ptr<HardwareContext>(aubManager.createHardwareContext(0, ENGINE_RCS, 0));
    hwContext->initialize();
}

TEST(AubManager, givenAubManagerWhenCallingAddCommentRedirectsToAubStream) {
    MockAubManager aubManager(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    MockAubFileStream *stream = new MockAubFileStream;
    const char *message = "comment";

    EXPECT_CALL(*stream, addComment(message)).Times(1);

    aubManager.streamAub.reset(stream);
    aubManager.addComment(message);
}

TEST(AubManager, givenLocalMemorySupportFalseWhenGpuRequiresLocalMemoryThenPageTablesAreInLocalMemory) {
    MockGpu mockGpu;
    bool localMemorySupport = false;

    EXPECT_CALL(mockGpu, requireLocalMemoryForPageTables()).Times(2).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(mockGpu, allocatePPGTT(_, MemoryBank::MEMORY_BANK_0, _)).Times(1);
    EXPECT_CALL(mockGpu, allocatePPGTT(_, MemoryBank::MEMORY_BANK_1, _)).Times(1);

    MockAubManager aubManager(mockGpu, 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
}

TEST(AubManager, givenGfxAllocationWhenFreeMemoryIsCalledThenFreeEachPpgtt) {
    MockAubManager aubManager(*gpu, 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    MockAubFileStream *stream = new MockAubFileStream;
    aubManager.streamAub.reset(stream);

    uint64_t gfxAddr = 0x1000000;
    uint64_t size = 0x10000;

    EXPECT_EQ(4u, aubManager.ppgtts.size());
    for (auto &ppgtt : aubManager.ppgtts) {
        EXPECT_CALL(*stream, freeMemory(ppgtt.get(), gfxAddr, size)).Times(1);
    }

    aubManager.freeMemory(gfxAddr, size);
}

TEST(AubManager, givenAubManagerMapsGpuVaEachPPGTTIsMapped) {
    MockAubManager aubManager(*gpu, 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    MockAubFileStream *stream = new MockAubFileStream;
    aubManager.streamAub.reset(stream);

    uint64_t gfxAddr = 0x1000000;
    uint64_t size = 0x10000;
    uint64_t physicalAddress = 0x8000;

    PhysicalAllocationInfo physicalParams = {physicalAddress, size, MEMORY_BANK_0, defaultPageSize};

    EXPECT_EQ(4u, aubManager.ppgtts.size());
    for (auto &ppgtt : aubManager.ppgtts) {
        EXPECT_CALL(*stream, mapGpuVa(ppgtt.get(), _, physicalAddress)).Times(1);
    }

    aubManager.mapGpuVa(gfxAddr, size, physicalParams);
}
