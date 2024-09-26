/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/physical_address_allocator.h"
#include "test_defaults.h"
#include "tests/unit_tests/mock_aub_manager.h"
#include "tests/unit_tests/mock_aub_stream.h"

#include <memory>

namespace aub_stream {

class HardwareContextFixture : public MockAubStreamFixture {
  public:
    void SetUp() {
        MockAubStreamFixture::SetUp();
        aubManager = std::make_unique<::testing::NiceMock<MockAubManager>>(createGpuFunc(), 1, defaultHBMSizePerDevice, 0u, true, mode::aubFile);
    }

    void TearDown() {
        MockAubStreamFixture::TearDown();
    }

    PhysicalAddressAllocatorSimple allocator;
    std::unique_ptr<::testing::NiceMock<MockAubManager>> aubManager;
};

class HardwareContextTest : public HardwareContextFixture, public ::testing::Test {
  public:
    void SetUp() override {
        HardwareContextFixture::SetUp();
    }

    void TearDown() override {
        HardwareContextFixture::TearDown();
    }
};
} // namespace aub_stream
