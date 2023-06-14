/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#if defined(_WIN32)
#include <malloc.h>
#else
#include <stdlib.h>
#endif

namespace aub_stream {

inline void *aligned_alloc(size_t s, size_t alignement) {
#if defined(_WIN32)
    return _aligned_malloc(s, alignement);
#else
    void *p = nullptr;
    if (posix_memalign(&p, alignement, s)) {
        p = nullptr;
    }
    return p;
#endif
}

inline void aligned_free(void *p) {
#if defined(_WIN32)
    _aligned_free(p);
#else
    free(p);
#endif
}

} // namespace aub_stream