/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "settings_reader.h"

namespace aub_stream {

int32_t EnvironmentReader::getSetting(const char *settingName, int32_t defaultValue) {
    return static_cast<int32_t>(getSetting(settingName, static_cast<int64_t>(defaultValue)));
}

int64_t EnvironmentReader::getSetting(const char *settingName, int64_t defaultValue) {
    int64_t value = defaultValue;
    char *envValue;

    envValue = os_calls::getEnv(settingName);
    if (envValue) {
        value = atoll(envValue);
    }
    return value;
}

bool EnvironmentReader::getSetting(const char *settingName, bool defaultValue) {
    return getSetting(settingName, static_cast<int64_t>(defaultValue)) ? true : false;
}

std::string EnvironmentReader::getSetting(const char *settingName, const std::string &value) {
    char *envValue;
    std::string keyValue;
    keyValue.assign(value);

    envValue = os_calls::getEnv(settingName);
    if (envValue) {
        keyValue.assign(envValue);
    }
    return keyValue;
}

} // namespace aub_stream
