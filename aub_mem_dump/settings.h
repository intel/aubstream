/*
 * Copyright (C) 2023 Intel Corporation
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

    void readSettings() {
#undef DECLARE_SETTING_VARIABLE
#define DECLARE_SETTING_VARIABLE(dataType, variableName, defaultValue, description)                 \
    {                                                                                               \
        dataType tempData = reader->getSetting(AUBSTREAM_PREFIX(variableName), variableName.get()); \
        variableName.set(tempData);                                                                 \
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
#define DECLARE_SETTING_VARIABLE(dataType, variableName, defaultValue, description)                   \
    {                                                                                                 \
        if (variableName.get() != defaultValue) {                                                     \
            changedSettings << AUBSTREAM_PREFIX(variableName) << " = " << variableName.get() << "\n"; \
        }                                                                                             \
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

} // namespace aub_stream
