/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>

namespace aub_stream {

enum class CoreFamily : uint32_t {
    Gen12lp = 3,
    XeHpCore = 4,
    XeHpgCore = 5,
    XeHpcCore = 6,
    Xe2HpgCore = 7,
    Xe3Core = 8,
    Xe3pCore = 9,
    MaxCore,
};

} // namespace aub_stream
