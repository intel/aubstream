/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/debug_helpers.h"
#include "aub_mem_dump/os_interface/os_calls.h"
#include "aub_mem_dump/settings.h"

#include <cstdio>

namespace aub_stream {

void handleDebugAssertFailure(const char *expression, const char *file, int line) {
    if (!globalSettings->EnableDebugAsserts.get()) {
        return;
    }
    fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", expression, file, line);
    fflush(stderr);
    if (os_calls::breakIntoDebugger()) {
        return;
    }
    os_calls::abortProcess();
}

} // namespace aub_stream
