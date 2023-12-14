/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdlib>

namespace aub_stream {
namespace os_calls {
using getenvPtr = decltype(&getenv);
extern getenvPtr getEnv;
} // namespace os_calls

} // namespace aub_stream
