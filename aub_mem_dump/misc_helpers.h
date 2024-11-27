/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <type_traits>

namespace aub_stream {

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

namespace TypeTraits {
template <typename T>
constexpr bool isPodV = std::is_standard_layout_v<T> && std::is_trivial_v<T> && std::is_trivially_copyable_v<T>;
}

} // namespace aub_stream
