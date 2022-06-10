/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "gtest/gtest.h"
#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "test_defaults.h"

namespace aub_stream {

void addSimpleBatchBuffer(HardwareContext *context, uint32_t memoryBank);
} // namespace aub_stream
