/*
 * Copyright (C) 2023-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace mock_os_calls {

extern std::unordered_map<std::string, std::string> environmentStrings;
extern bool breakHandledByDebugger;
extern uint32_t breakIntoDebuggerCalled;
extern uint32_t abortProcessCalled;

char *getEnv(const char *name) noexcept;
bool breakIntoDebugger();
void abortProcess();

void replaceCalls();

} // namespace mock_os_calls
