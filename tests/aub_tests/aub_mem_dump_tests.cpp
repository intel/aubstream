/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_file_stream.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_services.h"
#include "headers/aubstream.h"
#include "test_defaults.h"
#include "gtest/gtest.h"

#include "test.h"

using namespace aub_stream;

using CompareOperationValues = CmdServicesMemTraceMemoryCompare::CompareOperationValues;

template <typename Type>
struct AubTests : public ::testing::Test {
    using PPGTTType = Type;

    void SetUp() override;
    void TearDown() override;

    AubFileStream stream;
    PhysicalAddressAllocator allocator;
    GGTT *ggtt = nullptr;
    PPGTTType *ppgtt = nullptr;

    void writeVerifyOneByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize);
    void writeVerifySevenBytes(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize);
    void writeVerifyOneMegaByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize);
    void writeVerifyNotEqualOneByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize);
    void writeVerifyNotEqualSevenBytes(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize);
    void writeVerifyNotEqualOneMegaByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize);
};

template <typename PPGTTType>
struct AubTwoMemoryBanksTests : public AubTests<PPGTTType> {

    void SetUp() override {
        AubTests<PPGTTType>::SetUp();

        multiBankAllocator = std::make_unique<PhysicalAddressAllocator>(2, 2, localMemorySupportedInTests);

        ppgtt1 = std::make_unique<PPGTTType>(*gpu, multiBankAllocator.get(), defaultMemoryBank);
        ppgtt2 = std::make_unique<PPGTTType>(*gpu, multiBankAllocator.get(), defaultMemoryBank + 1);
    }
    void TearDown() override {
        AubTests<PPGTTType>::TearDown();
    }

    std::unique_ptr<PhysicalAddressAllocator> multiBankAllocator;
    std::unique_ptr<PPGTTType> ppgtt1;
    std::unique_ptr<PPGTTType> ppgtt2;
};

template <typename PPGTTType>
void AubTests<PPGTTType>::SetUp() {
    ggtt = new GGTT(*gpu, &allocator, defaultMemoryBank);
    ppgtt = new PPGTTType(*gpu, &allocator, defaultMemoryBank);

    initializeAubStream(stream);
}

template <typename PPGTTType>
void AubTests<PPGTTType>::TearDown() {
    delete ppgtt;
    delete ggtt;
}

template <typename PPGTTType>
void AubTests<PPGTTType>::writeVerifyOneByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize) {
    uint8_t data = 0xbf;
    auto context = 0xf00;
    SurfaceInfo surfaceInfo = {gfxAddress, sizeof(data), 1, sizeof(data), 0x1ff, surftype::buffer, tilingType::linear, false, dumpType::bin, false, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    stream.declareContextForDumping(context, ppgtt);
    if (pageTableType == AubStream::PAGE_TABLE_PPGTT) {
        stream.writeMemory(ppgtt, {gfxAddress, &data, sizeof(data), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize});
        stream.expectMemory(ppgtt, gfxAddress, &data, sizeof(data), CompareOperationValues::CompareEqual);
    } else {
        stream.writeMemory(ggtt, GGTT::AddressType(gfxAddress), &data, sizeof(data), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), &data, sizeof(data), CompareOperationValues::CompareEqual);
    }

    if (gfxAddress < std::numeric_limits<uint64_t>::max() - pageSize) {
        stream.dumpBufferBIN(pageTableType, gfxAddress, sizeof(data), context);
        stream.dumpSurface(pageTableType, surfaceInfo, context);
    }
}

template <typename PPGTTType>
void AubTests<PPGTTType>::writeVerifySevenBytes(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto context = 0xf00;
    SurfaceInfo surfaceInfo = {gfxAddress, sizeof(bytes), 1, sizeof(bytes), 0x1ff, surftype::buffer, tilingType::linear, false, dumpType::bin, false, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    stream.declareContextForDumping(context, ppgtt);
    if (pageTableType == AubStream::PAGE_TABLE_PPGTT) {
        stream.writeMemory(ppgtt, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize});
        stream.expectMemory(ppgtt, gfxAddress, bytes, sizeof(bytes), CompareOperationValues::CompareEqual);
    } else {
        stream.writeMemory(ggtt, GGTT::AddressType(gfxAddress), bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), bytes, sizeof(bytes), CompareOperationValues::CompareEqual);
    }

    if (gfxAddress < std::numeric_limits<uint64_t>::max() - pageSize) {
        stream.dumpBufferBIN(pageTableType, gfxAddress, sizeof(bytes), context);
        stream.dumpSurface(pageTableType, surfaceInfo, context);
    }
}

template <typename PPGTTType>
void AubTests<PPGTTType>::writeVerifyOneMegaByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize) {
    auto size = 1 * MB;
    auto bytes = std::make_unique<uint8_t[]>(size);
    auto context = 0xf00;
    SurfaceInfo surfaceInfo = {gfxAddress, size, 1, size, 0x1ff, surftype::buffer, tilingType::linear, false, dumpType::bin, false, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    stream.declareContextForDumping(context, ppgtt);
    if (pageTableType == AubStream::PAGE_TABLE_PPGTT) {
        stream.writeMemory(ppgtt, {gfxAddress, bytes.get(), size, defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize});
        stream.expectMemory(ppgtt, gfxAddress, bytes.get(), size, CompareOperationValues::CompareEqual);
    } else {
        stream.writeMemory(ggtt, GGTT::AddressType(gfxAddress), bytes.get(), size, defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), bytes.get(), size, CompareOperationValues::CompareEqual);
    }
    if (gfxAddress < std::numeric_limits<uint64_t>::max() - pageSize - size) {
        stream.dumpBufferBIN(pageTableType, gfxAddress, size, context);
        stream.dumpSurface(pageTableType, surfaceInfo, context);
    }
}

template <typename PPGTTType>
void AubTests<PPGTTType>::writeVerifyNotEqualOneByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize) {
    uint8_t data = 0xbf;
    uint8_t invalidData = 0xfb;
    SurfaceInfo surfaceInfo = {gfxAddress, sizeof(data), 1, sizeof(data), 0x1ff, surftype::buffer, tilingType::linear, false, dumpType::bin, false, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    auto context = 0xf00;

    stream.declareContextForDumping(context, ppgtt);
    if (pageTableType == AubStream::PAGE_TABLE_PPGTT) {
        stream.writeMemory(ppgtt, {gfxAddress, &data, sizeof(data), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize});
        stream.expectMemory(ppgtt, gfxAddress, &invalidData, sizeof(invalidData), CompareOperationValues::CompareNotEqual);
    } else {
        stream.writeMemory(ggtt, GGTT::AddressType(gfxAddress), &data, sizeof(data), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), &invalidData, sizeof(invalidData), CompareOperationValues::CompareNotEqual);
    }
    stream.dumpBufferBIN(pageTableType, gfxAddress, sizeof(data), context);
    stream.dumpSurface(pageTableType, surfaceInfo, context);
}

template <typename PPGTTType>
void AubTests<PPGTTType>::writeVerifyNotEqualSevenBytes(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint8_t invalidBytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'X'};
    auto context = 0xf00;
    SurfaceInfo surfaceInfo = {gfxAddress, sizeof(bytes), 1, sizeof(bytes), 0x1ff, surftype::buffer, tilingType::linear, false, dumpType::bin, false, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    stream.declareContextForDumping(context, ppgtt);
    if (pageTableType == AubStream::PAGE_TABLE_PPGTT) {
        stream.writeMemory(ppgtt, {gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize});
        stream.expectMemory(ppgtt, gfxAddress, invalidBytes, sizeof(invalidBytes), CompareOperationValues::CompareNotEqual);
    } else {
        stream.writeMemory(ggtt, GGTT::AddressType(gfxAddress), bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), invalidBytes, sizeof(invalidBytes), CompareOperationValues::CompareNotEqual);
    }
    stream.dumpBufferBIN(pageTableType, gfxAddress, sizeof(bytes), context);
    stream.dumpSurface(pageTableType, surfaceInfo, context);
}

template <typename PPGTTType>
void AubTests<PPGTTType>::writeVerifyNotEqualOneMegaByte(AubFileStream &stream, AubStream::PageTableType pageTableType, uint64_t gfxAddress, size_t pageSize) {
    using DataType = uint8_t;
    auto count = 1 * MB;
    auto size = count * sizeof(DataType);
    auto bytes = std::make_unique<DataType[]>(count);
    DataType item = 5u;
    std::fill_n(bytes.get(), size, item);
    auto invalidBytes1 = std::make_unique<DataType[]>(count);
    auto invalidBytes2 = std::make_unique<DataType[]>(count);
    auto invalidBytes3 = std::make_unique<DataType[]>(count);
    auto invalidBytes4 = std::make_unique<DataType[]>(count);
    memcpy(invalidBytes1.get(), bytes.get(), size);
    memcpy(invalidBytes2.get(), bytes.get(), size);
    memcpy(invalidBytes3.get(), bytes.get(), size);
    memcpy(invalidBytes4.get(), bytes.get(), size);
    invalidBytes1.get()[0]--;
    invalidBytes2.get()[0]++;
    invalidBytes3.get()[count - 1]--;
    invalidBytes4.get()[count - 1]++;
    auto context = 0xf00;

    stream.declareContextForDumping(context, ppgtt);
    if (pageTableType == AubStream::PAGE_TABLE_PPGTT) {
        stream.writeMemory(ppgtt, {gfxAddress, bytes.get(), size, defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize});
        stream.expectMemory(ppgtt, gfxAddress, invalidBytes1.get(), size, CompareOperationValues::CompareNotEqual);
        stream.expectMemory(ppgtt, gfxAddress, invalidBytes2.get(), size, CompareOperationValues::CompareNotEqual);
        stream.expectMemory(ppgtt, gfxAddress, invalidBytes3.get(), size, CompareOperationValues::CompareNotEqual);
        stream.expectMemory(ppgtt, gfxAddress, invalidBytes4.get(), size, CompareOperationValues::CompareNotEqual);
    } else {
        stream.writeMemory(ggtt, GGTT::AddressType(gfxAddress), bytes.get(), size, defaultMemoryBank, DataTypeHintValues::TraceNotype, pageSize);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), invalidBytes1.get(), size, CompareOperationValues::CompareNotEqual);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), invalidBytes2.get(), size, CompareOperationValues::CompareNotEqual);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), invalidBytes3.get(), size, CompareOperationValues::CompareNotEqual);
        stream.expectMemory(ggtt, GGTT::AddressType(gfxAddress), invalidBytes4.get(), size, CompareOperationValues::CompareNotEqual);
    }
    stream.dumpBufferBIN(pageTableType, gfxAddress, size, context);
}

using Aub48 = AubTests<PML4>;

TEST_F(Aub48, writeVerifyOneByteGGTT) {
    GGTT::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyOneByte(stream, AubStream::PAGE_TABLE_GGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyOneBytePPGTT4KB) {
    PPGTTType::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyOneByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyOneBytePPGTT64KB) {
    PPGTTType::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyOneByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeVerifySevenBytesGGTT) {
    GGTT::AddressType gfxAddress = (1ul << 12) - sizeof(uint32_t);
    writeVerifySevenBytes(stream, AubStream::PAGE_TABLE_GGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifySevenBytesPPGTT4KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifySevenBytes(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifySevenBytesPPGTT64KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifySevenBytes(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeVerifyOneMegaByteGGTT) {
    GGTT::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyOneMegaByte(stream, AubStream::PAGE_TABLE_GGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyOneMegaBytePPGTT4KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifyOneMegaByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyOneMegaBytePPGTT64KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifyOneMegaByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeVerifyNotEqualOneByteGGTT) {
    GGTT::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyNotEqualOneByte(stream, AubStream::PAGE_TABLE_GGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyNotEqualOneBytePPGTT4KB) {
    PPGTTType::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyNotEqualOneByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyNotEqualOneBytePPGTT64KB) {
    PPGTTType::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyNotEqualOneByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeVerifyNotEqualSevenBytesGGTT) {
    GGTT::AddressType gfxAddress = (1ul << 12) - sizeof(uint32_t);
    writeVerifyNotEqualSevenBytes(stream, AubStream::PAGE_TABLE_GGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyNotEqualSevenBytesPPGTT4KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifyNotEqualSevenBytes(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyNotEqualSevenBytesPPGTT64KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifyNotEqualSevenBytes(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeVerifyNotEqualOneMegaByteGGTT) {
    GGTT::AddressType gfxAddress = 0xbadddadcu;
    writeVerifyNotEqualOneMegaByte(stream, AubStream::PAGE_TABLE_GGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyNotEqualOneMegaBytePPGTT4KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifyNotEqualOneMegaByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeVerifyNotEqualOneMegaBytePPGTT64KB) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    writeVerifyNotEqualOneMegaByte(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeMaxAddress4KB) {
    PPGTTType::AddressType gfxAddress = static_cast<uintptr_t>(-1) - 7;
    writeVerifySevenBytes(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 4096);
}

TEST_F(Aub48, writeMaxAddress64KB) {
    PPGTTType::AddressType gfxAddress = static_cast<uintptr_t>(-1) - 7;
    writeVerifySevenBytes(stream, AubStream::PAGE_TABLE_PPGTT, gfxAddress, 65536);
}

TEST_F(Aub48, writeAndFreeMemory) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O', 'O', 'C', 'L', 0, 'N', 'E', 'O'};
    uint8_t bytes2[] = {'N', 'E', 'O', 0, 'O', 'C', 'L', 'N', 'E', 'O', 0, 'O', 'C', 'L'};
    const size_t fullSize = sizeof(bytes);
    const size_t halfSize = sizeof(bytes) / 2;

    auto gfxAddress = 0x2000 - halfSize;
    auto context = 0xf00;

    stream.declareContextForDumping(context, ppgtt);

    stream.writeMemory(ppgtt, {gfxAddress, bytes, fullSize, systemMemoryBank, DataTypeHintValues::TraceNotype, 4096});
    stream.expectMemory(ppgtt, gfxAddress, bytes, fullSize, CompareOperationValues::CompareEqual);

    stream.writeMemory(ppgtt, {gfxAddress + halfSize, &bytes2[halfSize], halfSize, systemMemoryBank, DataTypeHintValues::TraceNotype, 4096});

    stream.freeMemory(ppgtt, gfxAddress, halfSize);
    stream.expectMemory(ppgtt, gfxAddress + halfSize, &bytes2[halfSize], halfSize, CompareOperationValues::CompareEqual);

    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress + halfSize, halfSize, context);
}

using Aub48TwoMemoryBanksTests = AubTwoMemoryBanksTests<PML4>;

HWTEST_F(Aub48TwoMemoryBanksTests, writeAcrossMultipleBanks, MatchMultiDevice::moreThanOne) {
    // Choose a GPU address that straddles first level's boundary
    PPGTTType::AddressType gfxAddress = ppgtt->getNumAddressBits() == 48
                                            ? (1ull << 39) - sizeof(uint32_t)
                                            : (1ull << 30) - sizeof(uint32_t);
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto context = 0xf00;

    stream.writeMemory(ppgtt1.get(), {gfxAddress, bytes, sizeof(bytes), 3, DataTypeHintValues::TraceNotype, defaultPageSize});
    stream.expectMemory(ppgtt1.get(), gfxAddress, bytes, sizeof(bytes), CompareOperationValues::CompareEqual);
    stream.declareContextForDumping(context, ppgtt1.get());
    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress, sizeof(bytes), context);
}

HWTEST_F(Aub48TwoMemoryBanksTests, clonedPpgtt, MatchMultiDevice::moreThanOne) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto context1 = 0xf00;
    auto context2 = 0xf01;
    auto gfxAddress = 0xbadddadcu;

    PageTable *ppgttForCloning[] = {ppgtt2.get()};
    stream.declareContextForDumping(context1, ppgtt1.get());
    stream.declareContextForDumping(context2, ppgtt2.get());
    stream.writeMemoryAndClonePageTables(ppgtt1.get(), ppgttForCloning, 1, gfxAddress, bytes, sizeof(bytes), defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536);
    stream.expectMemory(ppgtt2.get(), gfxAddress, bytes, sizeof(bytes), CompareOperationValues::CompareEqual);

    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress, sizeof(bytes), context1);
    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress, sizeof(bytes), context2);
}

HWTEST_F(Aub48TwoMemoryBanksTests, memoryBankWrittenDifferentThanPpgttBank, MatchMultiDevice::moreThanOne) {
    uint8_t bytes[] = {'O', 'C', 'L', 0, 'N', 'E', 'O', 'O', 'C', 'L', 0, 'N', 'E', 'O'};
    auto context1 = 0xf00;
    auto context2 = 0xf01;

    uint64_t gfxAddress = 0x20'0000;
    auto gfxAddress1 = gfxAddress - (sizeof(bytes) / 2);

    PageTable *ppgttForCloning[] = {ppgtt1.get()};
    stream.declareContextForDumping(context1, ppgtt1.get());
    stream.declareContextForDumping(context2, ppgtt2.get());

    stream.writeMemoryAndClonePageTables(ppgtt2.get(), ppgttForCloning, 1, gfxAddress1, bytes, sizeof(bytes) / 2, defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536);
    stream.writeMemory(ppgtt1.get(), {gfxAddress, bytes, sizeof(bytes) / 2, defaultMemoryBank, DataTypeHintValues::TraceNotype, 65536});

    stream.expectMemory(ppgtt1.get(), gfxAddress1, bytes, sizeof(bytes), CompareOperationValues::CompareEqual);
    stream.expectMemory(ppgtt2.get(), gfxAddress1, bytes, sizeof(bytes) / 2, CompareOperationValues::CompareEqual);

    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress1, sizeof(bytes), context1);
    stream.dumpBufferBIN(AubStream::PAGE_TABLE_PPGTT, gfxAddress1, sizeof(bytes) / 2, context2);
}

HWTEST_F(Aub48, expectMemoryLarge64KBHBM, MatchMemory::hasBank0) {
    auto gfxAddress = 0xbadddadcu;
    size_t sizeBuffer = 0x100001;
    auto buffer = new uint8_t[sizeBuffer];

    for (size_t index = 0; index < sizeBuffer; ++index) {
        buffer[index] = static_cast<uint8_t>(index);
    }

    stream.writeMemory(ppgtt, {gfxAddress, buffer, sizeBuffer, MEMORY_BANK_0, DataTypeHintValues::TraceNotype, 65536});
    stream.expectMemory(ppgtt, gfxAddress, buffer, sizeBuffer, CompareOperationValues::CompareEqual);

    delete[] buffer;
}

TEST_F(Aub48, expectMemoryLarge4KBSystem) {
    auto gfxAddress = 0xbadddadcu;
    size_t sizeBuffer = 0x100001;
    auto buffer = new uint8_t[sizeBuffer];

    for (size_t index = 0; index < sizeBuffer; ++index) {
        buffer[index] = static_cast<uint8_t>(index);
    }

    stream.writeMemory(ppgtt, {gfxAddress, buffer, sizeBuffer, MEMORY_BANK_SYSTEM, DataTypeHintValues::TraceNotype, 4096});
    stream.expectMemory(ppgtt, gfxAddress, buffer, sizeBuffer, CompareOperationValues::CompareEqual);

    delete[] buffer;
}
