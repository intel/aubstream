/*
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aub_mem_dump/settings_reader.h"

#include <cstdint>
#include <string>
#include <sstream>
#include <memory>
#include <iostream>

namespace aub_stream {
#define STRINGIFY(var) #var
#define AUBSTREAM_PREFIX(var) STRINGIFY(AUBSTREAM_##var)

#if defined AUBSTREAM_SETTINGS_DISABLE
constexpr bool enableSettings = false;
#else
constexpr bool enableSettings = true;
#endif

struct LogLevels {
    constexpr static int32_t info{1};
    constexpr static int32_t error{1 << 1};
    constexpr static int32_t verbose{1 << 2};
};

class Settings;
extern Settings *globalSettings;

template <typename T>
struct SettingVar {
    SettingVar(const T &defaultValue) : value(defaultValue), defaultValue(defaultValue) {}
    T get() const {
        return value;
    }
    void set(T data) {
        value = std::move(data);
    }

  private:
    T value;
    T defaultValue;
};

class Settings {
  public:
    virtual ~Settings() = default;
    Settings(std::ostream *out = nullptr) : outStream(out ? *out : std::cout) {
        reader = std::make_unique<EnvironmentReader>();
        readSettings();
        printSettings();
    }

    static constexpr bool disabled() {
        return !enableSettings;
    }

    static constexpr const char *getSettingName(const char *variable) {
        return (disabled()) ? "" : variable;
    }

    void readSettings() {
#undef DECLARE_SETTING_VARIABLE
#define DECLARE_SETTING_VARIABLE(dataType, variableName, defaultValue, description)                                 \
    {                                                                                                               \
        dataType tempData = reader->getSetting(getSettingName(AUBSTREAM_PREFIX(variableName)), variableName.get()); \
        variableName.set(tempData);                                                                                 \
    }

#include "setting_vars.inl"
#undef DECLARE_SETTING_VARIABLE
    }

    void printSettings() {
        if (!PrintSettings.get()) {
            return;
        }

        std::ostringstream changedSettings;
#undef DECLARE_SETTING_VARIABLE
#define DECLARE_SETTING_VARIABLE(dataType, variableName, defaultValue, description)                                   \
    {                                                                                                                 \
        if (variableName.get() != defaultValue && getSettingName(AUBSTREAM_PREFIX(variableName))) {                   \
            changedSettings << getSettingName(AUBSTREAM_PREFIX(variableName)) << " = " << variableName.get() << "\n"; \
        }                                                                                                             \
    }

#include "setting_vars.inl"
#undef DECLARE_SETTING_VARIABLE
        outStream << changedSettings.str();
    }

    std::unique_ptr<SettingsReader> reader;
    std::ostream &outStream;
#define DECLARE_SETTING_VARIABLE(dataType, variableName, defaultValue, description) \
    SettingVar<dataType> variableName{defaultValue};
#include "setting_vars.inl"
#undef DECLARE_SETTING_VARIABLE
};

#define PRINT_LOG(str, ...) \
    printf(str, __VA_ARGS__);

#define PRINT_LOG_VERBOSE(str, ...)                            \
    if (globalSettings->LogLevel.get() & LogLevels::verbose) { \
        PRINT_LOG("[VERBOSE] " str, __VA_ARGS__);              \
    }

} // namespace aub_stream
