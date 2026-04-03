/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tests/test_traits/test_traits.h"

namespace aub_stream {
EnableTestTraits<ProductFamily::Nvlp> enableNvlp(NVLP_CONFIG);
} // namespace aub_stream
