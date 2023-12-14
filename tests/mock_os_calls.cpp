/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/os_interface/os_calls.h"

#include <string>
#include <unordered_map>

namespace mock_os_calls {

std::unordered_map<std::string, std::string> environmentStrings;

char *getEnv(const char *name) noexcept {
    if (const auto it = environmentStrings.find(name); it != environmentStrings.end()) {
        return const_cast<char *>(it->second.c_str());
    }
    return nullptr;
}

void replaceCalls() {
    aub_stream::os_calls::getEnv = mock_os_calls::getEnv;
}
} // namespace mock_os_calls
