/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#ifndef WIN32

#include <errno.h>
#include <cstring>

inline int memcpy_s(void *dst, size_t destSize, const void *src, size_t count) {
    if ((dst == nullptr) || (src == nullptr)) {
        return -EINVAL;
    }
    if (destSize < count) {
        return -ERANGE;
    }

    memcpy(dst, src, count);

    return 0;
}

#endif
