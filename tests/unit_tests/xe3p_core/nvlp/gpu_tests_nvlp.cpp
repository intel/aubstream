/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "test_defaults.h"
#include "gtest/gtest.h"
#include "aub_mem_dump/memory_banks.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include "gmock/gmock.h"
#include "test.h"

using namespace aub_stream;
using ::testing::_;

TEST(Gpu, gpuNvlpReturnsCorrectDeviceId) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Nvlp);

    EXPECT_EQ(0u, gpu->deviceId);
}

TEST(Nvlp, allocatePML5) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Nvlp);
    PhysicalAddressAllocatorSimple allocator;

    auto ppgtt = std::unique_ptr<PageTable>(gpu->allocatePPGTT(&allocator, MEMORY_BANK_0, maxNBitValue(57)));
    EXPECT_EQ(57u, ppgtt->getNumAddressBits());
}

TEST(Nvlp, givenNvlpWhenInitializingGlobalMmiosThenProgramMocsAndPatIndex) {
    TEST_REQUIRES(gpu->productFamily == ProductFamily::Nvlp);
    MockAubFileStream stream;
    EXPECT_CALL(stream, writeMMIO(_, _, 0xffffffff)).Times(::testing::AtLeast(0));

    EXPECT_CALL(stream, writeMMIO(0x4000, 0b0000'0000'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4004, 0b0001'0000'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4008, 0b0001'0011'0000, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x400C, 0b0001'0011'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4010, 0b0001'0000'0000, 0xffffffff)).Times(1);

    // PAT
    EXPECT_CALL(stream, writeMMIO(0x4800, 0b0000'0000'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4804, 0b0000'0000'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4808, 0b0000'0000'1111, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x480C, 0b0000'0011'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4810, 0b0000'0011'0010, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4814, 0b0000'0011'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4818, 0b0100'0001'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x481C, 0b0000'0011'0011, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4848, 0b0000'0011'0000, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x484C, 0b0010'0000'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4850, 0b0010'0011'0000, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4854, 0b0110'0001'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4858, 0b0010'0011'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x485C, 0b0000'0000'0000, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4860, 0b0010'0000'0000, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4864, 0b0110'0001'0100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4868, 0b0010'0000'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4870, 0b0100'0010'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4874, 0b0100'0010'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4878, 0b0000'0100'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x487C, 0b0010'0100'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4880, 0b0000'0100'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4884, 0b0000'0100'1111, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4888, 0b0000'1000'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x488C, 0b0010'1000'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4890, 0b0000'1000'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4894, 0b0000'1000'1111, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4898, 0b0000'1100'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x489C, 0b0010'1100'1100, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x48A0, 0b0000'1100'1110, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x48A4, 0b0000'1100'1111, 0xffffffff)).Times(1);

    EXPECT_CALL(stream, writeMMIO(0x47FC, 0b0000'0000'1111, 0xffffffff)).Times(1);
    EXPECT_CALL(stream, writeMMIO(0x4820, 0b0000'0000'0011, 0xffffffff)).Times(1);

    gpu->initializeGlobalMMIO(stream, 1, 1, 0);
}
