/*
 * Copyright (C) 2023-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdlib>

namespace aub_stream {
namespace os_calls {
using getenvPtr = decltype(&getenv);
using breakIntoDebuggerPtr = bool (*)();
using abortProcessPtr = void (*)();

extern getenvPtr getEnv;
extern breakIntoDebuggerPtr breakIntoDebugger;
extern abortProcessPtr abortProcess;
} // namespace os_calls

} // namespace aub_stream
