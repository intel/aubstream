/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aub_mem_dump/debug_helpers.h"
#include "aub_mem_dump/os_interface/os_calls.h"
#include "aub_mem_dump/settings.h"
#include "tests/mock_os_calls.h"
#include "tests/variable_backup.h"

#include <cstdint>
#include <memory>

constexpr uint32_t expectedCallsWhenDebugAssertsCompiledIn = aub_stream::debugAssertsCompiledIn ? 1u : 0u;

struct DebugAssertFixture {
    void SetUp() {
        mock_os_calls::breakHandledByDebugger = false;
        mock_os_calls::breakIntoDebuggerCalled = 0;
        mock_os_calls::abortProcessCalled = 0;
        aub_stream::globalSettings = settings.get();
    }

    void TearDown() {
    }

    std::unique_ptr<aub_stream::Settings> settings = std::make_unique<aub_stream::Settings>();
    VariableBackup<aub_stream::Settings *> settingsBackup{&aub_stream::globalSettings};
    VariableBackup<aub_stream::os_calls::breakIntoDebuggerPtr> breakIntoDebuggerBackup{&aub_stream::os_calls::breakIntoDebugger, mock_os_calls::breakIntoDebugger};
    VariableBackup<aub_stream::os_calls::abortProcessPtr> abortProcessBackup{&aub_stream::os_calls::abortProcess, mock_os_calls::abortProcess};
};
