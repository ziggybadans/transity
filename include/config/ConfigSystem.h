#pragma once

#include <string>
#include <map>
#include <any>

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
        auto it = configValues.find(key);
        if (it != configValues.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast& e) {
                LOG_ERROR(("Failed to cast config value for key: " + key + " to type: " + typeid(T).name()).c_str(), "Config");
            }
        }
        return defaultValue;
    }
private:
    std::map<std::string, std::any> configValues;
};

}