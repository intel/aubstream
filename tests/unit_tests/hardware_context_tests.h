/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/physical_address_allocator.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include <memory>

namespace aub_stream {

class HardwareContextTest : public MockAubStreamFixture, public ::testing::Test {
  public:
    void SetUp() override {
        MockAubStreamFixture::SetUp();
        aubManager = std::make_unique<::testing::NiceMock<MockAubManager>>(*gpu, 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    }

    void TearDown() override {
        MockAubStreamFixture::TearDown();
    }

    PhysicalAddressAllocator allocator;
    std::unique_ptr<::testing::NiceMock<MockAubManager>> aubManager;
};
} // namespace aub_stream
