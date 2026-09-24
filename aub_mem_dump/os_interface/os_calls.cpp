/*
 * Copyright (C) 2023-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <cstdlib>
#include "os_calls.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <csignal>
#endif

namespace aub_stream {
namespace os_calls {

namespace {

#if defined(_WIN32)
bool breakIntoDebuggerImpl() {
    __try {
        __debugbreak();
    } __except (GetExceptionCode() == EXCEPTION_BREAKPOINT ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        return false;
    }
    return true;
}
#else
volatile sig_atomic_t breakpointReachedProcess = 0;

void onBreakpointSignal(int) {
    breakpointReachedProcess = 1;
}

bool breakIntoDebuggerImpl() {
    breakpointReachedProcess = 0;
    auto previousHandler = std::signal(SIGTRAP, onBreakpointSignal);
    std::raise(SIGTRAP);
    std::signal(SIGTRAP, previousHandler);
    return breakpointReachedProcess == 0;
}
#endif

void abortProcessImpl() {
    std::abort();
}

} // namespace

getenvPtr getEnv = getenv;
breakIntoDebuggerPtr breakIntoDebugger = breakIntoDebuggerImpl;
abortProcessPtr abortProcess = abortProcessImpl;

} // namespace os_calls

} // namespace aub_stream
