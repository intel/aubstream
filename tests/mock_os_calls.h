/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <string>
#include <unordered_map>

namespace mock_os_calls {

extern std::unordered_map<std::string, std::string> environmentStrings;
char *getEnv(const char *name) noexcept;

void replaceCalls();

} // namespace mock_os_calls
