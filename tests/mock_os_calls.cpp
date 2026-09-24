/*
 * Copyright (C) 2023-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tests/mock_os_calls.h"

#include "aub_mem_dump/os_interface/os_calls.h"

#include <string>
#include <unordered_map>

namespace mock_os_calls {

std::unordered_map<std::string, std::string> environmentStrings;
bool breakHandledByDebugger = false;
uint32_t breakIntoDebuggerCalled = 0;
uint32_t abortProcessCalled = 0;

char *getEnv(const char *name) noexcept {
    if (const auto it = environmentStrings.find(name); it != environmentStrings.end()) {
        return const_cast<char *>(it->second.c_str());
    }
    return nullptr;
}

bool breakIntoDebugger() {
    breakIntoDebuggerCalled++;
    return breakHandledByDebugger;
}

void abortProcess() {
    abortProcessCalled++;
}

void replaceCalls() {
    aub_stream::os_calls::getEnv = mock_os_calls::getEnv;
}
} // namespace mock_os_calls
