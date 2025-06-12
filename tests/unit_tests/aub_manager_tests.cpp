/*
 * Copyright (C) 2022-2025 Intel Corporation
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
#include "aub_mem_dump/options.h"

#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/unit_tests/mock_aub_stream.h"
#include "tests/unit_tests/mock_physical_address_allocator.h"
#include "tests/unit_tests/mock_gpu.h"
#include "tests/unit_tests/page_table_helper.h"

#include "test.h"

using namespace aub_stream;
using ::testing::_;
using ::testing::AtLeast;

TEST(AubManagerTest, givenNotSupportedGenWhenAubManagerIsCreatedUsingOldInterfaceThenNullptrIsReturnedUsingOptions) {
    AubManagerOptions internalOptions;
    internalOptions.version = 0;
    internalOptions.productFamily = 0;
    internalOptions.devicesCount = defaultDeviceCount;
    internalOptions.memoryBankSize = defaultHBMSizePerDevice;
    internalOptions.stepping = defaultStepping;
    internalOptions.localMemorySupported = true;
    internalOptions.mode = mode::aubFile;
    internalOptions.gpuAddressSpace = maxNBitValue(48);
    auto aubManager = AubManager::create(internalOptions);

    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenNotSupportedGenWhenAubManagerIsCreatedThenNullptrIsReturnedUsingOptions) {
    AubManagerOptions internalOptions;
    internalOptions.version = 1;
    internalOptions.productFamily = static_cast<uint32_t>(ProductFamily::MaxProduct);
    internalOptions.devicesCount = defaultDeviceCount;
    internalOptions.memoryBankSize = defaultHBMSizePerDevice;
    internalOptions.stepping = defaultStepping;
    internalOptions.localMemorySupported = true;
    internalOptions.mode = mode::aubFile;
    internalOptions.gpuAddressSpace = maxNBitValue(48);
    auto aubManager = AubManager::create(internalOptions);

    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenSupportedProductFamilyWhenAubManagerIsCreatedThenValidPtrIsReturned) {

    AubManagerOptions internalOptions;
    internalOptions.version = 1;
    internalOptions.productFamily = static_cast<uint32_t>(gpu->productFamily);
    internalOptions.devicesCount = defaultDeviceCount;
    internalOptions.memoryBankSize = defaultHBMSizePerDevice;
    internalOptions.stepping = defaultStepping;
    internalOptions.localMemorySupported = true;
    internalOptions.mode = mode::aubFile;
    internalOptions.gpuAddressSpace = maxNBitValue(48);
    auto aubManager = AubManager::create(internalOptions);

    EXPECT_NE(nullptr, aubManager);
    delete aubManager;
}

TEST(AubManagerImp, givenNullStreamModeWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsTrue) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, aub_stream::mode::null);
    aubManager.initialize();

    EXPECT_TRUE(aubManager.isInitialized());

    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamTbxShm.get());
    EXPECT_EQ(nullptr, aubManager.getStream());

    EXPECT_TRUE(aubManager.isOpen());
}

TEST(AubManagerImp, givenNullStreamModeWhenAubManagerIsCreatedThenAllMethodsThatRequireStreamReturnsEarlyWithoutCrash) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, aub_stream::mode::null);
    aubManager.initialize();
    auto hwContext = aubManager.createHardwareContext(0, 0, 0);
    EXPECT_NE(nullptr, hwContext);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    EXPECT_NO_THROW(aubManager.writeMemory2({gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, defaultPageSize}));
    std::vector<PageInfo> lastLevelPages{};
    EXPECT_NO_THROW(aubManager.writePageTableEntries(gfxAddress, sizeof(bytes), MEMORY_BANK_0, 0, lastLevelPages, defaultPageSize));
    EXPECT_NO_THROW(aubManager.writePhysicalMemoryPages(bytes, lastLevelPages, sizeof(bytes), 0));
    EXPECT_NO_THROW(aubManager.freeMemory(gfxAddress, sizeof(bytes)));
    PhysicalAllocationInfo physicalParams = {gfxAddress, sizeof(bytes), MEMORY_BANK_0, defaultPageSize};
    EXPECT_NO_THROW(aubManager.reservePhysicalMemory({gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, defaultPageSize}, physicalParams));
    EXPECT_NO_THROW(aubManager.reserveOnlyPhysicalSpace({gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, defaultPageSize}, physicalParams));

    EXPECT_NO_THROW(aubManager.writeMMIO(0x1234, 0x5678));
    EXPECT_NO_THROW(aubManager.writePCICFG(0x1234, 0x5678));
    EXPECT_EQ(0u, aubManager.readMMIO(0x1234));
    EXPECT_EQ(0u, aubManager.readPCICFG(0x1234));
}

TEST(AubManagerImp, givenInvalidStreamModeWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, std::numeric_limits<uint32_t>::max());
    aubManager.initialize();

    EXPECT_FALSE(aubManager.isInitialized());

    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamTbxShm.get());
}

TEST(AubManagerImp, givenInvalidStreamModeWhenAubManagerCreateCalledThenNullptrReturned) {
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

    auto aubManager = AubManager::create(options);
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

TEST(AubManagerImp, givenInvalidSHMPointersForSHMModeAndExceptionsEnabledWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = true;

    EXPECT_THROW(AubManager::create(options), std::logic_error);
}

TEST(AubManagerImp, givenInvalidSHMPointersForSHMModeWhenAubManagerCreateCalledThenNullptrReturned) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;

    auto aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerImp, givenInvalidSHM4PointersForSHMModeAndExceptionsEnabledWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm4;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = true;

    EXPECT_THROW(AubManager::create(options), std::logic_error);
}

TEST(AubManagerImp, givenInvalidSHM4PointersForSHMModeWhenAubManagerCreateCalledThenNullptrReturned) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm4;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;

    auto aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithAubFileModeAndOpenIsCalledThenItInitializesAubFileStream) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.initialize();

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
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbx);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_TRUE(static_cast<TbxStream *>(aubManager.streamTbx.get())->socket);

    EXPECT_FALSE(aubManager.isOpen());
    EXPECT_TRUE(aubManager.getFileName().empty());
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithTbxModeThenItInitializesTbxShmStream) {
    uint8_t sysMem[0x1000];
    uint8_t lMem[0x1000];
    SharedMemoryInfo sharedMemoryInfo = {sysMem, 0x1000, lMem, 0x1000};
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbxShm, sharedMemoryInfo);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamTbxShm.get());
    EXPECT_TRUE(static_cast<TbxShmStream *>(aubManager.streamTbxShm.get())->socket);
    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
    EXPECT_FALSE(aubManager.isOpen());
    EXPECT_TRUE(aubManager.getFileName().empty());
}

TEST(AubManagerImp, whenAubManagerIsCreatedWithAubFileAndTbxModeThenItInitializesAubAndTbxStreams) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFileAndTbx);
    aubManager.initialize();

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
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, stepping, true, mode::aubFile);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamAub.get());

    EXPECT_EQ(stepping, aubManager.stepping);
}

TEST(AubManagerImp, whenSetCCSModeWith1CCSCountIsCalledThenProperMMIOIsWritten) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.createStream();
    auto &stream = *aubManager.getMockAubFileStream();
    aubManager.initialize();

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x14804, 0xFFF0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset, 0xFFF0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 2, 0xFFF0000)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 3, 0xFFF0000)).Times(1);

    aubManager.setCCSMode(1);
}

TEST(AubManagerImp, whenSetCCSModeWith2CCSCountIsCalledThenProperMMIOIsWritten) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.createStream();
    auto &stream = *aubManager.getMockAubFileStream();
    aubManager.initialize();

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x14804, 0xFFF0240)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset, 0xFFF0240)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 2, 0xFFF0240)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 3, 0xFFF0240)).Times(1);

    aubManager.setCCSMode(2);
}

TEST(AubManagerImp, whenSetCCSModeWith4CCSCountIsCalledThenProperMMIOIsWritten) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.createStream();
    auto &stream = *aubManager.getMockAubFileStream();
    aubManager.initialize();

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x14804, 0xFFF0688)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset, 0xFFF0688)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 2, 0xFFF0688)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 3, 0xFFF0688)).Times(1);

    aubManager.setCCSMode(4);
}

TEST(AubManagerImp, whenSetCCSModeWithLargeCCSCountIsCalledThenDefaultMMIOIsWritten) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.createStream();
    auto &stream = *aubManager.getMockAubFileStream();
    aubManager.initialize();

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0x14804, 0xFFF0688)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset, 0xFFF0688)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 2, 0xFFF0688)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x14804 + mmioDeviceOffset * 3, 0xFFF0688)).Times(1);

    aubManager.setCCSMode(8);
}

TEST(AubManagerImp, whenSetCCSModeIsCalledAfterHwContextCreationAndEnableThrowIsTrueThenExceptionIsThrown) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile, {}, true);
    aubManager.createStream();
    aubManager.initialize();
    aubManager.createHardwareContext(0, 0, 0);

    EXPECT_THROW(aubManager.setCCSMode(1), std::logic_error);
    EXPECT_THROW(aubManager.setCCSMode(2), std::logic_error);
    EXPECT_THROW(aubManager.setCCSMode(4), std::logic_error);
}

TEST(AubManagerImp, whenSetCCSModeIsCalledAfterHwContextCreationAndEnableThrowIsFalseThenExceptionIsNotThrown) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile, {}, false);
    aubManager.createStream();
    aubManager.initialize();
    aubManager.createHardwareContext(0, 0, 0);

    EXPECT_NO_THROW(aubManager.setCCSMode(1));
    EXPECT_NO_THROW(aubManager.setCCSMode(2));
    EXPECT_NO_THROW(aubManager.setCCSMode(4));
}

using AubManagerTest = ::testing::Test;
HWTEST_F(AubManagerTest, whenAubManagerIsCreatedWithTbxModeThenItInitializesTbxShm3Stream, HwMatcher::coreAboveEqualXeHp) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbxShm3);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamTbxShm.get());
    EXPECT_TRUE(static_cast<TbxShmStream *>(aubManager.streamTbxShm.get())->socket);
    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
    EXPECT_FALSE(aubManager.isOpen());
    EXPECT_TRUE(aubManager.getFileName().empty());
}

HWTEST_F(AubManagerTest, whenAubManagerIsCreatedWithTbxModeThenItInitializesTbxShm4Stream, HwMatcher::coreAboveEqualXeHp) {
    constexpr size_t BankSize = 0x1000000;
    uint8_t *sysMem[BankSize / 0x1000] = {0};
    uint8_t *lMem[BankSize / 0x1000] = {0};
    SharedMemoryInfo sharedMemoryInfo = {reinterpret_cast<uint8_t *>(sysMem), BankSize, reinterpret_cast<uint8_t *>(lMem), BankSize};
    MockAubManager aubManager(createGpuFunc(), 1, BankSize, 0u, gpu->requireLocalMemoryForPageTables(), mode::tbxShm4, sharedMemoryInfo);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamTbxShm.get());
    EXPECT_TRUE(static_cast<TbxShmStream *>(aubManager.streamTbxShm.get())->socket);
    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
    EXPECT_FALSE(aubManager.isOpen());
    EXPECT_TRUE(aubManager.getFileName().empty());
    if (gpu->requireLocalMemoryForPageTables()) {
        EXPECT_NE(aubManager.translatePhysicalAddressToSystemMemory(BankSize - 0x1000, true), nullptr);
        EXPECT_EQ(aubManager.translatePhysicalAddressToSystemMemory(BankSize - 0x1000, true), lMem[BankSize / 0x1000 - 1]);
        EXPECT_EQ(aubManager.translatePhysicalAddressToSystemMemory(0, true), nullptr);
    } else {
        EXPECT_NE(aubManager.translatePhysicalAddressToSystemMemory(BankSize - 0x1000, false), nullptr);
        EXPECT_EQ(aubManager.translatePhysicalAddressToSystemMemory(BankSize - 0x1000, false), sysMem[BankSize / 0x1000 - 1]);
        EXPECT_EQ(aubManager.translatePhysicalAddressToSystemMemory(0, false), nullptr);
    }
}

HWTEST_F(AubManagerTest, givenInvalidSHMPointersForSHM3ModeAndExceptionsEnabledWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse, HwMatcher::coreAboveEqualXeHp) {
    uint8_t sysMem[0x1000];
    uint8_t lMem[0x1000];
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm3;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = true;
    options.sharedMemoryInfo = {sysMem, 0x1000, lMem, 0x1000};

    EXPECT_THROW(AubManager::create(options), std::logic_error);

    options.sharedMemoryInfo = {nullptr, 0, lMem, 0x1000};
    EXPECT_THROW(AubManager::create(options), std::logic_error);

    options.sharedMemoryInfo = {sysMem, 0x1000, nullptr, 0};
    EXPECT_THROW(AubManager::create(options), std::logic_error);
}

HWTEST_F(AubManagerTest, givenInvalidSHMPointersForSHM3ModeWhenAubManagerCreateCalledThenNullptrReturned, HwMatcher::coreAboveEqualXeHp) {
    uint8_t sysMem[0x1000];
    uint8_t lMem[0x1000];
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm3;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;
    options.sharedMemoryInfo = {sysMem, 0x1000, lMem, 0x1000};

    auto aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);

    options.sharedMemoryInfo = {nullptr, 0, lMem, 0x1000};
    aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);

    options.sharedMemoryInfo = {sysMem, 0x1000, nullptr, 0};
    aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

HWTEST_F(AubManagerTest, givenInvalidProductFamilyForSHM3AndExceptionsEnabledWhenAubManagerIsCreatedThenNoStreamIsCreatedAndIsInitializedReturnsFalse, HwMatcher::coreEqualGen12Core) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm3;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = true;
    options.sharedMemoryInfo = {nullptr, 0x1000, nullptr, 0x1000};

    EXPECT_THROW(AubManager::create(options), std::logic_error);
}

HWTEST_F(AubManagerTest, givenInvalidProductFamilyForSHM3WhenAubManagerCreateCalledThenNullptrReturned, HwMatcher::coreEqualGen12Core) {
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm3;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;
    options.sharedMemoryInfo = {nullptr, 0x1000, nullptr, 0x1000};

    auto aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenTooMuchStolenMemorySizeWhenAubManagerCreateCalledThenNullptrReturned) {
    uint8_t sysMem[0x1000];
    uint8_t lMem[0x1000];
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;
    options.sharedMemoryInfo = {sysMem, 0x1000, lMem, 0x1000};
    options.dataStolenMemorySize = defaultHBMSizePerDevice;
    auto aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

TEST(AubManagerTest, givenInvalidStolenMemorySizeWhenAubManagerCreateCalledThenNullptrReturned) {
    uint8_t sysMem[0x1000];
    uint8_t lMem[0x1000];
    AubManagerOptions options;
    options.version = 1;
    options.productFamily = static_cast<uint32_t>(gpu->productFamily);
    options.devicesCount = defaultDeviceCount;
    options.memoryBankSize = defaultHBMSizePerDevice;
    options.stepping = defaultStepping;
    options.localMemorySupported = true;
    options.mode = mode::tbxShm;
    options.gpuAddressSpace = maxNBitValue(48);
    options.throwOnError = false;
    options.sharedMemoryInfo = {sysMem, 0x1000, lMem, 0x1000};
    options.dataStolenMemorySize = 4 * MB + 1;
    auto aubManager = AubManager::create(options);
    EXPECT_EQ(nullptr, aubManager);
}

HWTEST_F(AubManagerTest, ggttBaseAddressIsCorrect, HwMatcher::coreAboveEqualXeHp) {

    bool localMemorySupport = false;
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    auto sm = StolenMemory::CreateStolenMemory(false, 1, defaultHBMSizePerDevice, 4 * MB);

    EXPECT_EQ(1u, aubManager.ggtts.size());
    if (!gpu->requireLocalMemoryForPageTables()) {
        EXPECT_EQ(gpu->getGGTTBaseAddress(0, defaultHBMSizePerDevice, sm->getBaseAddress(0)), aubManager.ggtts[0].get()->gttTableOffset);
    }
}

HWTEST_F(AubManagerTest, when2DevicesAreCreatedThenAubManagerIsInitializedCorrectly, MatchMultiDevice::moreThanOne) {

    bool localMemorySupport = false;
    MockAubManager aubManager(createGpuFunc(), 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

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
    MockAubManager aubManager(createGpuFunc(), 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    EXPECT_EQ(2u, aubManager.ppgtts.size());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_0, aubManager.ppgtts[0].get()->getMemoryBank());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_1, aubManager.ppgtts[1].get()->getMemoryBank());

    EXPECT_EQ(2u, aubManager.ggtts.size());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_0, aubManager.ggtts[0].get()->getMemoryBank());
    EXPECT_EQ(MemoryBank::MEMORY_BANK_1, aubManager.ggtts[1].get()->getMemoryBank());

    EXPECT_NE(nullptr, aubManager.physicalAddressAllocator.get());
    EXPECT_NE(nullptr, aubManager.streamAub.get());
}

TEST(AubManagerImp, createHardwareContextShouldReturnValidHardwareContext) {
    bool localMemorySupport = true;
    MockAubManager aubManager(createGpuFunc(), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    auto hardwareContext = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

    EXPECT_NE(nullptr, hardwareContext);
    delete hardwareContext;
}

TEST(AubManagerImp, releaseHardwareContextRemovesContextFromVectorAndDeletesObject) {
    bool localMemorySupport = true;
    MockAubManager aubManager(createGpuFunc(), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    auto hardwareContext = aubManager.createHardwareContext(0, ENGINE_RCS, 0);

    EXPECT_NE(nullptr, hardwareContext);
    EXPECT_EQ(1u, aubManager.hwContexts.size());

    EXPECT_TRUE(aubManager.releaseHardwareContext(hardwareContext));

    EXPECT_EQ(0u, aubManager.hwContexts.size());
    EXPECT_FALSE(aubManager.releaseHardwareContext(hardwareContext));
}

HWTEST_F(AubManagerTest, whenAubManagerWritesMemoryThenPageTablesCloned, MatchMultiDevice::moreThanOne) {

    bool localMemorySupport = true;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    MockAubManager aubManager(createGpuFunc(), 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    aubManager.writeMemory2({gfxAddress, bytes, sizeof(bytes), MEMORY_BANK_0, 0, defaultPageSize});

    auto physicalAddress1 = PageTableHelper::getEntry(aubManager.ppgtts[0].get(), gfxAddress);
    auto physicalAddress2 = PageTableHelper::getEntry(aubManager.ppgtts[1].get(), gfxAddress);
    EXPECT_EQ(physicalAddress1, physicalAddress2);
}

HWTEST_F(AubManagerTest, whenAubManagerWritesMemoryThenPageTablesParamsCloned, MatchMultiDevice::moreThanOne) {
    bool localMemorySupport = true;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint64_t gfxAddress = 0x1000;

    MockAubManager aubManager(createGpuFunc(), 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

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

TEST(AubManagerImp, whenAubManagerWritesPageTableEntiesThenPhysicalMemory) {
    bool localMemorySupport = false;
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto gfxAddress = 0x1000;
    std::vector<PageInfo> lastLevelentries;

    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    aubManager.writePageTableEntries(gfxAddress, sizeof(bytes), MEMORY_BANK_SYSTEM, 0, lastLevelentries, defaultPageSize);

    EXPECT_EQ(lastLevelentries.size(), 1);
}

TEST(AubManagerImp, initializeAlsoInitializesGlobalMmio) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    auto deviceCount = mockGpu->deviceCount;
    EXPECT_CALL(*mockGpu, initializeGlobalMMIO(_, deviceCount, defaultHBMSizePerDevice, 0u));

    MockAubManager aubManager(std::move(mockGpu), deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();
    aubManager.open("test.aub");
}

TEST(AubManagerImp, initializeInjectsMMIOsLast) {
    MMIOListInjected.push_back(MMIOPair(0xABCD, 0x20002));
    MMIOListInjected.push_back(MMIOPair(0xDEAD, 0x20002));
    MMIOListInjected.push_back(MMIOPair(0x20d8, 0x1111));

    MockAubManager aubManager(createGpuFunc(), gpu->deviceCount, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.createMockAubFileStream = true;
    aubManager.createStream();
    auto &stream = *aubManager.getMockAubFileStream();

    uint32_t regAddr = 0;
    uint32_t regVal = 0;

    EXPECT_CALL(stream, writeMMIO(_, _)).Times(AtLeast(0));
    EXPECT_CALL(stream, writeMMIO(0xABCD, 0x20002)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0xDEAD, 0x20002)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x20d8, _)).Times(AtLeast(1)).WillRepeatedly(::testing::Invoke([&](uint32_t registerOffset, uint32_t value) { 
            regAddr = registerOffset; 
            regVal = value; }));

    aubManager.initialize();
    aubManager.open("test.aub");

    EXPECT_EQ(0x1111u, regVal);

    MMIOListInjected.resize(0);
}

TEST(AubManager, initializeAlsoSetsMemoryBankSize) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(*mockGpu, setMemoryBankSize(_, gpu->deviceCount, defaultHBMSizePerDevice));

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();
    aubManager.open("test.aub");
}

TEST(AubManager, initializeAlsoSetsGGTTBaseAddresses) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(*mockGpu, setGGTTBaseAddresses(_, gpu->deviceCount, defaultHBMSizePerDevice, _));

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::tbx);
    aubManager.initialize();
    aubManager.open("test.aub");
}

TEST(AubManager, initializeAllocatesPageTablesFromGpu) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    EXPECT_CALL(*mockGpu, allocatePPGTT(_, _, _)).Times(gpu->deviceCount);
    EXPECT_CALL(*mockGpu, allocateGGTT(_, _, _)).Times(gpu->deviceCount);

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();
}

TEST(AubManager, givenAubManagerCreatedWithAubFileModeWhenGetStreamIsCalledThenAubFileStreamIsReturned) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    EXPECT_EQ(aubManager.streamAub.get(), aubManager.getStream());
}

TEST(AubManager, givenAubManagerCreatedWithTbxModeWhenGetStreamIsCalledThenTbxStreamIsReturned) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::tbx);
    aubManager.initialize();
    EXPECT_EQ(aubManager.streamTbx.get(), aubManager.getStream());
}

TEST(AubManager, givenAubManagerCreatedWithAubFileAndTbxModeWhenGetStreamIsCalledThenAubTbxStreamIsReturned) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFileAndTbx);
    aubManager.initialize();
    EXPECT_EQ(aubManager.streamAubTbx.get(), aubManager.getStream());
}

TEST(AubManager, givenAubFileModeWhenCreatingAubManagerThenOnlyAubFileStreamIsCreated) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
}

TEST(AubManager, givenTbxModeWhenCreatingAubManagerThenOnlyTbxStreamIsCreated) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::tbx);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_EQ(nullptr, aubManager.streamAub.get());
    EXPECT_EQ(nullptr, aubManager.streamAubTbx.get());
}

TEST(AubManager, givenAubFileAndTbxModeWhenCreatingAubManagerThenAllStreamsAreCreated) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;

    MockAubManager aubManager(std::move(mockGpu), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFileAndTbx);
    aubManager.initialize();

    EXPECT_NE(nullptr, aubManager.streamAub.get());
    EXPECT_NE(nullptr, aubManager.streamTbx.get());
    EXPECT_NE(nullptr, aubManager.streamAubTbx.get());
}

TEST(AubManager, givenAubManagerCreatedWithAubFileAndTbxModeWhenHardwareContextIsCreatedThenAubTbxStreamIsUsed) {
    bool localMemorySupport = defaultMemoryBank != MEMORY_BANK_SYSTEM;
    MockAubManager aubManager(createGpuFunc(), gpu->deviceCount, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFileAndTbx);
    aubManager.initialize();

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
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    MockAubFileStream *stream = new MockAubFileStream;
    const char *message = "comment";

    EXPECT_CALL(*stream, addComment(message)).Times(1);

    aubManager.streamAub.reset(stream);
    aubManager.addComment(message);
}

TEST(AubManager, givenLocalMemorySupportFalseWhenGpuRequiresLocalMemoryThenPageTablesAreInLocalMemory) {
    auto mockGpu = std::make_unique<MockGpu>();
    bool localMemorySupport = false;

    EXPECT_CALL(*mockGpu, requireLocalMemoryForPageTables()).Times(2).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*mockGpu, allocatePPGTT(_, MemoryBank::MEMORY_BANK_0, _)).Times(1);
    EXPECT_CALL(*mockGpu, allocatePPGTT(_, MemoryBank::MEMORY_BANK_1, _)).Times(1);

    MockAubManager aubManager(std::move(mockGpu), 2, defaultHBMSizePerDevice, 0u, localMemorySupport, mode::aubFile);
    aubManager.initialize();
}

TEST(AubManager, givenGfxAllocationWhenFreeMemoryIsCalledThenFreeEachPpgtt) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.initialize();
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

MATCHER_P(EqAllocationParams, x, "Matcher for type AllocationParams") {
    return std::tie(arg.gfxAddress, arg.size, arg.memoryBanks, arg.additionalParams.compressionEnabled) == std::tie(x.gfxAddress, x.size, x.memoryBanks, x.additionalParams.compressionEnabled);
}

TEST(AubManager, givenAubManagerMapsGpuVaEachPPGTTIsMapped) {
    MockAubManager aubManager(createGpuFunc(), 4, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    aubManager.initialize();
    MockAubFileStream *stream = new MockAubFileStream;
    aubManager.streamAub.reset(stream);

    uint64_t gfxAddr = 0x1000000;
    uint64_t size = 0x10000;
    uint64_t physicalAddress = 0x8000;

    PhysicalAllocationInfo physicalParams = {physicalAddress, size, MEMORY_BANK_0, defaultPageSize};
    AllocationParams allocationParams(gfxAddr, nullptr, size, MEMORY_BANK_0, 0, defaultPageSize);
    allocationParams.additionalParams.compressionEnabled = false;

    EXPECT_EQ(4u, aubManager.ppgtts.size());
    for (auto &ppgtt : aubManager.ppgtts) {
        EXPECT_CALL(*stream, mapGpuVa(ppgtt.get(), EqAllocationParams(allocationParams), physicalAddress)).Times(1);
    }

    aubManager.mapGpuVa(gfxAddr, size, physicalParams);

    gfxAddr += size;
    allocationParams.gfxAddress = gfxAddr;
    allocationParams.additionalParams.compressionEnabled = false;
    allocationParams.additionalParams.uncached = false;
    for (auto &ppgtt : aubManager.ppgtts) {
        EXPECT_CALL(*stream, mapGpuVa(ppgtt.get(), EqAllocationParams(allocationParams), physicalAddress)).Times(1);
    }

    aubManager.mapGpuVa2(physicalAddress, allocationParams);

    gfxAddr += size;
    allocationParams.gfxAddress = gfxAddr;
    allocationParams.additionalParams.compressionEnabled = true;
    allocationParams.additionalParams.uncached = true;
    for (auto &ppgtt : aubManager.ppgtts) {
        EXPECT_CALL(*stream, mapGpuVa(ppgtt.get(), EqAllocationParams(allocationParams), physicalAddress)).Times(1);
    }

    aubManager.mapGpuVa2(physicalAddress, allocationParams);
}

TEST(AubManager, givenAubManagerDoMMIOAndPCICFGOperations) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbx);
    aubManager.initialize();
    MockTbxStream *stream = new MockTbxStream;
    aubManager.streamTbx.reset(stream);

    EXPECT_CALL(*stream, readMMIO(0x1234)).Times(1);
    aubManager.readMMIO(0x1234);
    EXPECT_CALL(*stream, writeMMIO(0x4321, 0xabcd)).Times(1);
    aubManager.writeMMIO(0x4321, 0xabcd);
    EXPECT_CALL(*stream, readPCICFG(0x1234)).Times(1);
    aubManager.readPCICFG(0x1234);
    EXPECT_CALL(*stream, writePCICFG(0x4321, 0xabcd)).Times(1);
    aubManager.writePCICFG(0x4321, 0xabcd);
}

TEST(AubManager, givenAubManagerSHM4WhenCallingReservePhysicalMemoryRedirectsToPhysicalAllocator) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbxShm4);
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(1, defaultHBMSizePerDevice, true, nullptr);
    auto mockPhysicalAddressAllocatorSimpleAndSHM4Mapper = new MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(1, defaultHBMSizePerDevice, true, nullptr);
    aubManager.physicalAddressAllocator.reset(mockPhysicalAddressAllocatorSimpleAndSHM4Mapper);

    AllocationParams allocationParams(0, nullptr, 0x2000, MEMORY_BANK_SYSTEM, 0, 0x1000);
    PhysicalAllocationInfo physicalAllocInfo;
    EXPECT_CALL(*mockPhysicalAddressAllocatorSimpleAndSHM4Mapper, reservePhysicalMemory(MEMORY_BANK_SYSTEM, 0x2000, 0x1000)).Times(1);
    aubManager.reservePhysicalMemory(allocationParams, physicalAllocInfo);
}

TEST(AubManager, givenAubManagerSHM4WhenCallingReservePhysicalMemoryRedirectsToPhysicalAllocatorAndNotAllocateMemoryOnHeap) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbxShm4);
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(1, defaultHBMSizePerDevice, true, nullptr);
    auto mockPhysicalAddressAllocatorSimpleAndSHM4Mapper = new MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(1, defaultHBMSizePerDevice, true, nullptr);
    aubManager.physicalAddressAllocator.reset(mockPhysicalAddressAllocatorSimpleAndSHM4Mapper);

    AllocationParams allocationParams(0, nullptr, 0x2000, MEMORY_BANK_SYSTEM, 0, 0x1000);
    PhysicalAllocationInfo physicalAllocInfo;
    EXPECT_CALL(*mockPhysicalAddressAllocatorSimpleAndSHM4Mapper, reservePhysicalMemory(MEMORY_BANK_SYSTEM, 0x2000, 0x1000)).Times(0);
    aubManager.reserveOnlyPhysicalSpace(allocationParams, physicalAllocInfo);
    EXPECT_EQ(mockPhysicalAddressAllocatorSimpleAndSHM4Mapper->storage.size(), 0);
    EXPECT_EQ(physicalAllocInfo.physicalAddress, reservedGGTTSpace);
}

TEST(AubManager, givenAubManagerSHM4WhenCallingTranslatePhysicalAddressToSystemMemoryRedirectsToPhysicalAllocator) {
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::tbxShm4);
    MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(1, defaultHBMSizePerDevice, true, nullptr);
    auto mockPhysicalAddressAllocatorSimpleAndSHM4Mapper = new MockPhysicalAddressAllocatorSimpleAndSHM4Mapper(1, defaultHBMSizePerDevice, true, nullptr);
    aubManager.physicalAddressAllocator.reset(mockPhysicalAddressAllocatorSimpleAndSHM4Mapper);

    EXPECT_CALL(*mockPhysicalAddressAllocatorSimpleAndSHM4Mapper, translatePhysicalAddressToSystemMemory(0x2000, 0x1000, false, _, _)).Times(1);
    aubManager.translatePhysicalAddressToSystemMemory(0x2000, false);
}

TEST(AubManager, givenAubManagerSHMWhenCallingTranslatePhysicalAddressToSystemMemoryPointerOnSHMAreaIsReturned) {
    constexpr size_t BankSize = 0x3000;
    uint8_t sysMem[0x3000] = {0};
    uint8_t lMem[0x3000] = {0};
    SharedMemoryInfo sharedMemoryInfo = {sysMem, BankSize, lMem, BankSize};
    MockAubManager aubManager(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, gpu->requireLocalMemoryForPageTables(), mode::tbxShm, sharedMemoryInfo);
    aubManager.initialize();

    void *p = aubManager.translatePhysicalAddressToSystemMemory(0x2000, false);
    EXPECT_EQ(p, sysMem + 0x2000);
}
