/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstdint>

namespace aub_stream {
constexpr uint64_t toBitValue(uint32_t bit) {
    return static_cast<uint64_t>(1u) << static_cast<uint64_t>(bit);
}
template <typename... T>
inline uint64_t toBitValue(uint32_t bit, T... nextBits) {
    return static_cast<uint64_t>(1u) << static_cast<uint64_t>(bit) | toBitValue(nextBits...);
}

namespace PpgttEntryBits {
const uint32_t presentBit = 0;
const uint32_t writableBit = 1;
const uint32_t userSupervisorBit = 2;
const uint32_t intermediatePageSizeBit = 6;
const uint32_t largePageSizeBit = 7;
const uint32_t atomicEnableBit = 10;
const uint32_t legacyIntermediatePageSizeBit = 11;
const uint32_t localMemoryBit = 11;
} // namespace PpgttEntryBits
} // namespace aub_stream
