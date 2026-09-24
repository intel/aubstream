/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

namespace aub_stream {

#if defined(NDEBUG)
constexpr bool debugAssertsCompiledIn = false;
#else
constexpr bool debugAssertsCompiledIn = true;
#endif

void handleDebugAssertFailure(const char *expression, const char *file, int line);

} // namespace aub_stream

#define AUBSTREAM_DEBUG_ASSERT(expression)                                             \
    do {                                                                               \
        if constexpr (aub_stream::debugAssertsCompiledIn) {                            \
            if (!(expression)) {                                                       \
                aub_stream::handleDebugAssertFailure(#expression, __FILE__, __LINE__); \
            }                                                                          \
        }                                                                              \
    } while (false)
