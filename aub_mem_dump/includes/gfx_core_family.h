/*
 * Copyright (C) 2022-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>

namespace aub_stream {

enum class CoreFamily : uint32_t {
    Gen8,
    Gen9,
    Gen11,
    Gen12lp,
    XeHpCore,
    XeHpgCore,
    XeHpcCore,
    Xe2HpgCore,
    MaxCore,
};

} // namespace aub_stream
