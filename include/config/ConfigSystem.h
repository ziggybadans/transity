#pragma once

#include <string>
#include <map>
#include <any>

#include <toml++/toml.hpp>

#include "logging/LoggingSystem.hpp"

namespace transity::config {

class ConfigSystem {
public:
    ConfigSystem();
    ~ConfigSystem();

    void initialize(const std::string& primaryConfigFilepath = "config.toml",
                    const std::string& userConfigFilepath = "");

    template <typename T>
    T getValue(const std::string& key, T defaultValue) const {
        std::optional<T> result;

        if (userConfigTable) {
            if (auto node = userConfigTable->at_path(key)) {
                result = node.value<T>();
                if (result) {
                    return *result;
                }
            }
        }
        if (primaryConfigTable) {
            if (auto node = primaryConfigTable->at_path(key)) {
                result = node.value<T>();
                if (result) {
                    return *result;
                }
            }
        }
        if (auto node = defaultConfigValues.at_path(key)) {
            result = node.value<T>();
            if (result) {
                return *result;
            }
        }

        LOG_WARN(("Config key '" + key + "' not found or type mismatch in any config source. Returning default.").c_str(), "Config");
        return defaultValue;
    }

    inline std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        return getValue<std::string>(key, defaultValue);
    }

    inline int getInt(const std::string& key, int defaultValue = 0) const {
        return getValue<int>(key, defaultValue);
    }

    inline bool getBool(const std::string& key, bool defaultValue = false) const {
        return getValue<bool>(key, defaultValue);
    }

    inline double getDouble(const std::string& key, double defaultValue = 0.0) const {
        return getValue<double>(key, defaultValue);
    }

    inline float getFloat(const std::string& key, float defaultValue = 0.0f) const {
        return static_cast<float>(getValue<double>(key, static_cast<double>(defaultValue)));
    }
private:
    toml::table defaultConfigValues; // For hardcoded defaults
    std::optional<toml::table> primaryConfigTable; // Parsed primary file
    std::optional<toml::table> userConfigTable;    // Parsed user file
    // Maybe store file paths if needed for saving later
    std::string storedPrimaryPath;
    std::string storedUserPath; 
};

}