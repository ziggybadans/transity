#include "config/ConfigSystem.h"

#include <toml++/toml.hpp>

#include <filesystem>
#include <string>

namespace transity::config {

ConfigSystem::ConfigSystem() {
    initialize();
    LOG_INFO("Config system initialized", "Config");
}

ConfigSystem::~ConfigSystem() {
    LOG_INFO("Config system shutting down", "Config");
    configValues.clear();
}

void processTomlTable(const toml::table& tbl, const std::string& currentPrefix, std::map<std::string, std::any>& configMap) {
    for (auto&& [key, node] : tbl) {
        std::string fullKey = currentPrefix.empty() ? std::string(key) : currentPrefix + "." + std::string(key);
        if (node.is_table()) {
            processTomlTable(*node.as_table(), fullKey, configMap);
        } else if (node.is_value()) {
            if (auto val = node.value<std::string>()) {
                configMap[fullKey] = *val;
            } else if (auto val = node.value<int64_t>()) {
                configMap[fullKey] = static_cast<int>(*val);
            } else if (auto val = node.value<double>()) {
                configMap[fullKey] = *val;
            } else if (auto val = node.value<bool>()) {
                configMap[fullKey] = *val;
            }
        }
    }
}

void ConfigSystem::initialize(const std::string& primaryConfigFilepath, const std::string& userConfigFilepath) {
    configValues.clear();
    configValues["windowWidth"] = 800;
    configValues["windowHeight"] = 600;
    configValues["windowTitle"] = std::string("Transity");

    LOG_INFO("Default configuration loaded", "Config");

    LOG_INFO(("Attempting to load primary config from: " + primaryConfigFilepath).c_str(), "Config");
    if (std::filesystem::exists(primaryConfigFilepath)) {
        try {
            toml::table tbl = toml::parse_file(primaryConfigFilepath);
            processTomlTable(tbl, "", configValues);
            LOG_INFO(("Primary config loaded from: " + primaryConfigFilepath).c_str(), "Config");
        } catch (const toml::parse_error& err) {
            LOG_ERROR(("Failed to parse primary config from: " + primaryConfigFilepath + ". Using defaults.").c_str(), "Config");
        } catch (const std::exception& e) {
            LOG_ERROR(("Unexpected error while parsing primary config from: " + primaryConfigFilepath + ". Using defaults.").c_str(), "Config");
        }
    } else {
        LOG_WARN(("Primary config not found at: " + primaryConfigFilepath + ". Using defaults.").c_str(), "Config");
    }

    if (!userConfigFilepath.empty()) {
        LOG_INFO(("Attempting to load user config from: " + userConfigFilepath).c_str(), "Config");
        if (std::filesystem::exists(userConfigFilepath)) {
            try {
                toml::table tbl = toml::parse_file(userConfigFilepath);
                processTomlTable(tbl, "", configValues);
                LOG_INFO(("User config loaded from: " + userConfigFilepath).c_str(), "Config");
            } catch (const toml::parse_error& err) {
                LOG_ERROR(("Failed to parse user config from: " + userConfigFilepath + ". Ignoring.").c_str(), "Config");
            } catch (const std::exception& e) {
                LOG_ERROR(("Unexpected error while parsing user config from: " + userConfigFilepath + ". Ignoring.").c_str(), "Config");
            }
        } else {
            LOG_WARN(("User config not found at: " + userConfigFilepath + ". Ignoring.").c_str(), "Config");
        }
    } else {
        LOG_INFO("No user config file specified. Ignoring.", "Config");
    }
}

}