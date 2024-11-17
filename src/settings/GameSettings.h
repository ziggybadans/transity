#pragma once

#include <string>
#include "SettingsRegistry.h"

class GameSettings {
public:
    GameSettings() = default;
    ~GameSettings() = default;

    bool LoadSettings(const std::string& filepath);
    bool SaveSettings(const std::string& filepath) const;

    template<typename T>
    T GetValue(const std::string& name) const {
        return SettingsRegistry::Instance().GetValueAs<T>(name);
    }

    template<typename T>
    void SetValue(const std::string& name, const T& value) {
        SettingsRegistry::Instance().SetValue(name, value);
    }
}; 