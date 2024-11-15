/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tests/test_traits/test_traits.h"

namespace aub_stream {

EnableTestTraits<ProductFamily::Ptl> enablePtl(PTL_CONFIG);

} // namespace aub_stream
