/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace aub_stream {

template <typename T>
inline T ptrOffset(T ptrBefore, size_t offset) {
    const auto addrBefore = reinterpret_cast<uintptr_t>(ptrBefore);
    const auto addrAfter = addrBefore + offset;
    return reinterpret_cast<T>(addrAfter);
}

constexpr uint16_t countBits(uint16_t v) {
    uint16_t r = ((v >> 1) & 0x5555) + (v & 0x5555);
    r = ((r >> 2) & 0x3333) + (r & 0x3333);
    r = ((r >> 4) & 0x0f0f) + (r & 0x0f0f);
    r = ((r >> 8) & 0x0ff) + (r & 0x0ff);
    return r & 0xff;
}

template <typename T, size_t n>
constexpr size_t arrayCount(const T (&)[n]) {
    return n;
}

} // namespace aub_stream
