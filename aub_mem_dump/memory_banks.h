/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>
#include <string>

namespace aub_stream {

static const uint32_t KB = 1024ul;
static const uint32_t MB = 1024ul * KB;
static const uint32_t GB = 1024ull * MB;

enum MemoryBank : uint32_t {
    MEMORY_BANK_SYSTEM = 0,
    MEMORY_BANK_0 = 1,
    MEMORY_BANK_1 = 2,
    MEMORY_BANK_2 = 4,
    MEMORY_BANK_3 = 8,
    MEMORY_BANK_4 = 16,
    MEMORY_BANK_5 = 32,
    MEMORY_BANK_6 = 64,
    MEMORY_BANK_7 = 128,
    MEMORY_BANK_8 = 256,
    MEMORY_BANK_9 = 512,
    MEMORY_BANK_10 = 1024,
    MEMORY_BANK_11 = 2048,
    MEMORY_BANK_12 = 4096,
    MEMORY_BANK_13 = 8192,
    MEMORY_BANK_14 = 16384,
    MEMORY_BANK_15 = 32768,
    MEMORY_BANK_16 = 65536,
    MEMORY_BANK_17 = 131072,
    MEMORY_BANK_18 = 262144,
    MEMORY_BANK_19 = 524288,
    MEMORY_BANK_20 = 1048576,
    MEMORY_BANK_21 = 2097152,
    MEMORY_BANK_22 = 4194304,
    MEMORY_BANK_23 = 8388608,
    MEMORY_BANK_24 = 16777216,
    MEMORY_BANK_25 = 33554432,
    MEMORY_BANK_26 = 67108864,
    MEMORY_BANK_27 = 134217728,
    MEMORY_BANK_28 = 268435456,
    MEMORY_BANK_29 = 536870912,
    MEMORY_BANK_30 = 1073741824,
    MEMORY_BANK_31 = 2147483648,
    MEMORY_BANKS_ALL = 0xffffffff
};

inline uint32_t MEMORY_BANK(uint32_t device) {
    return MEMORY_BANK_0 << device;
}

std::string memoryBanksToString(uint32_t banks, const std::string &separator = "");

} // namespace aub_stream
