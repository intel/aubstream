/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>
#include <string>
#include "aub_mem_dump/os_interface/os_calls.h"

namespace aub_stream {

class SettingsReader {
  public:
    virtual ~SettingsReader() = default;
    virtual int32_t getSetting(const char *settingName, int32_t defaultValue) = 0;
    virtual int64_t getSetting(const char *settingName, int64_t defaultValue) = 0;
    virtual bool getSetting(const char *settingName, bool defaultValue) = 0;
    virtual std::string getSetting(const char *settingName, const std::string &value) = 0;
};

class EnvironmentReader : public SettingsReader {
  public:
    int32_t getSetting(const char *settingName, int32_t defaultValue) override;
    int64_t getSetting(const char *settingName, int64_t defaultValue) override;
    bool getSetting(const char *settingName, bool defaultValue) override;
    std::string getSetting(const char *settingName, const std::string &value) override;
};

} // namespace aub_stream
