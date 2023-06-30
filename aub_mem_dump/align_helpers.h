/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
namespace aub_stream {

constexpr uint64_t alignUp(uint64_t size, uint64_t alignementBits) {
    const uint64_t alignementVal = uint64_t(1) << alignementBits;
    const uint64_t alignementMask = alignementVal - 1;
    return (size + alignementMask) & ~alignementMask;
}
constexpr uint64_t alignDown(uint64_t address, uint64_t alignementBits) {
    const uint64_t alignementVal = uint64_t(1) << alignementBits;
    const uint64_t alignementMask = alignementVal - 1;
    return address & ~alignementMask;
}

} // namespace aub_stream
