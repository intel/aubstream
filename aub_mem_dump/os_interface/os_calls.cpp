/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <cstdlib>
#include "os_calls.h"

namespace aub_stream {
namespace os_calls {
getenvPtr getEnv = getenv;

}

} // namespace aub_stream
