/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tests/test_traits/test_traits.h"

namespace aub_stream {
EnableTestTraits<PRODUCT_FAMILY::IGFX_XE_HP_SDV> enableXeHpSdv(XE_HP_SDV_CONFIG);
} // namespace aub_stream