/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "mock_aub_stream.h"
#include "gtest/gtest.h"

#include <memory>
#pragma once

class CommandStreamerHelperTest : public aub_stream::MockAubStreamFixture, public ::testing::Test {
  public:
    bool checkLRIInLRCA(uint32_t *lrca, size_t sizeLrca, uint32_t mmioEngine, uint32_t address, uint32_t value) {
        bool lriFound = false;
        for (uint32_t i = 0; i < sizeLrca / sizeof(uint32_t); i += 2) {
            if (lrca[i] == (mmioEngine + address) && lrca[i + 1] == value) {
                lriFound = true;
                break;
            }
        }
        return lriFound;
    }

    void SetUp() override {
        aub_stream::MockAubStreamFixture::SetUp();
    }

    void TearDown() override {
        aub_stream::MockAubStreamFixture::TearDown();
    }
};
