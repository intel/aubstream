/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "aub_stream.h"
#include <string>
#include <cstdint>
#include <ostream>
#include <vector>

namespace aub_stream {

extern std::ostream &log;
extern std::string tbxServerIp;
extern uint16_t tbxServerPort;
extern bool tbxFrontdoorMode;
extern MMIOList MMIOListInjected;
extern uint32_t aubStreamCaller;

} // namespace aub_stream
