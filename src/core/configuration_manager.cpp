#include "transity/core/configuration_manager.hpp"
#include "transity/core/error.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

namespace transity::core {

ConfigurationManager& ConfigurationManager::getInstance() {
    static ConfigurationManager instance;
    return instance;
}

bool ConfigurationManager::initialize(const std::filesystem::path& configPath) {
    configFilePath = configPath;
    
    if (!loadConfiguration()) {
        spdlog::warn("No configuration file found at {}. Using default settings.", configPath.string());
        return true; // Not finding a config file is not an error - we'll create it when saving
    }

    if (!validateConfiguration()) {
        spdlog::error("Configuration validation failed");
        return false;
    }

    migrateConfiguration();
    return true;
}

void ConfigurationManager::shutdown() {
    if (isDirty) {
        if (!saveConfiguration()) {
            spdlog::error("Failed to save configuration during shutdown");
        }
    }
}

bool ConfigurationManager::hasKey(const std::string& key) const {
    return configData.find(key) != configData.end();
}

void ConfigurationManager::removeKey(const std::string& key) {
    if (configData.erase(key) > 0) {
        isDirty = true;
    }
}

bool ConfigurationManager::saveConfiguration() {
    try {
        nlohmann::json jsonConfig;

        // Convert configuration data to JSON
        for (const auto& [key, value] : configData) {
            std::visit([&](const auto& v) {
                jsonConfig[key] = v;
            }, value);
        }

        // Create directories if they don't exist
        if (auto parent = configFilePath.parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        // Write to file with pretty printing
        std::ofstream file(configFilePath);
        if (!file) {
            throw ConfigurationError("Failed to open configuration file for writing");
        }
        file << jsonConfig.dump(4);
        isDirty = false;
        return true;
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to save configuration: {}", e.what());
        return false;
    }
}

bool ConfigurationManager::loadConfiguration() {
    try {
        if (!std::filesystem::exists(configFilePath)) {
            return false;
        }

        std::ifstream file(configFilePath);
        if (!file) {
            throw ConfigurationError("Failed to open configuration file for reading");
        }

        nlohmann::json jsonConfig;
        file >> jsonConfig;

        // Clear existing configuration
        configData.clear();

        // Load new configuration
        for (auto it = jsonConfig.begin(); it != jsonConfig.end(); ++it) {
            const auto& key = it.key();
            const auto& value = it.value();

            if (value.is_boolean()) {
                configData[key] = value.get<bool>();
            }
            else if (value.is_number_integer()) {
                configData[key] = value.get<int>();
            }
            else if (value.is_number_float()) {
                configData[key] = value.get<double>();
            }
            else if (value.is_string()) {
                configData[key] = value.get<std::string>();
            }
            else {
                spdlog::warn("Skipping unsupported value type for key: {}", key);
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to load configuration: {}", e.what());
        return false;
    }
}

bool ConfigurationManager::validateConfiguration() {
    // TODO: Add specific validation rules as needed
    return true;
}

void ConfigurationManager::migrateConfiguration() {
    // TODO: Implement configuration version checking and migration
}

// Template specializations for getValue
template<>
bool ConfigurationManager::getValue<bool>(const std::string& key, const bool& defaultValue) const {
    auto it = configData.find(key);
    if (it == configData.end()) return defaultValue;
    return std::get<bool>(it->second);
}

template<>
int ConfigurationManager::getValue<int>(const std::string& key, const int& defaultValue) const {
    auto it = configData.find(key);
    if (it == configData.end()) return defaultValue;
    return std::get<int>(it->second);
}

template<>
double ConfigurationManager::getValue<double>(const std::string& key, const double& defaultValue) const {
    auto it = configData.find(key);
    if (it == configData.end()) return defaultValue;
    return std::get<double>(it->second);
}

template<>
std::string ConfigurationManager::getValue<std::string>(
    const std::string& key, const std::string& defaultValue) const {
    auto it = configData.find(key);
    if (it == configData.end()) return defaultValue;
    return std::get<std::string>(it->second);
}

// Template specializations for setValue
template<>
void ConfigurationManager::setValue<bool>(const std::string& key, const bool& value) {
    configData[key] = value;
    isDirty = true;
}

template<>
void ConfigurationManager::setValue<int>(const std::string& key, const int& value) {
    configData[key] = value;
    isDirty = true;
}

template<>
void ConfigurationManager::setValue<double>(const std::string& key, const double& value) {
    configData[key] = value;
    isDirty = true;
}

template<>
void ConfigurationManager::setValue<std::string>(const std::string& key, const std::string& value) {
    configData[key] = value;
    isDirty = true;
}

} // namespace transity::core 