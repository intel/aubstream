/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tests/test_traits/test_traits.h"

namespace aub_stream {
EnableTestTraits<PRODUCT_FAMILY::IGFX_TIGERLAKE_LP> enableTgllp(TGLLP_CONFIG);
} // namespace aub_stream