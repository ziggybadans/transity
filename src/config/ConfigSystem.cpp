#include "config/ConfigSystem.h"

#include <filesystem>
#include <string>

namespace transity::config {

ConfigSystem::ConfigSystem() {
    initialize();
    LOG_INFO("Config system initialized", "Config");
}

ConfigSystem::~ConfigSystem() {
    LOG_INFO("Config system shutting down", "Config");
    defaultConfigValues.clear();
}

void ConfigSystem::initialize(const std::string& primaryConfigFilepath, const std::string& userConfigFilepath) {
    defaultConfigValues.clear();
    defaultConfigValues.insert_or_assign("windowWidth", 800);
    defaultConfigValues.insert_or_assign("windowHeight", 600);
    defaultConfigValues.insert_or_assign("windowTitle", "Transity");

    LOG_INFO("Default configuration loaded", "Config");

    LOG_INFO(("Attempting to load primary config from: " + primaryConfigFilepath).c_str(), "Config");
    if (std::filesystem::exists(primaryConfigFilepath)) {
        try {
            primaryConfigTable = toml::parse_file(primaryConfigFilepath);
            storedPrimaryPath = primaryConfigFilepath;
            LOG_INFO(("Primary config loaded from: " + primaryConfigFilepath).c_str(), "Config");
        } catch (const toml::parse_error& err) {
            primaryConfigTable.reset();
            LOG_ERROR(("Failed to parse primary config from: " + primaryConfigFilepath + ". Using defaults.").c_str(), "Config");
        } catch (const std::exception& e) {
            primaryConfigTable.reset();
            LOG_ERROR(("Unexpected error while parsing primary config from: " + primaryConfigFilepath + ". Using defaults.").c_str(), "Config");
        }
    } else {
        LOG_WARN(("Primary config not found at: " + primaryConfigFilepath + ". Using defaults.").c_str(), "Config");
    }

    if (!userConfigFilepath.empty()) {
        LOG_INFO(("Attempting to load user config from: " + userConfigFilepath).c_str(), "Config");
        if (std::filesystem::exists(userConfigFilepath)) {
            try {
                userConfigTable = toml::parse_file(userConfigFilepath);
                storedUserPath = userConfigFilepath;
                LOG_INFO(("User config loaded from: " + userConfigFilepath).c_str(), "Config");
            } catch (const toml::parse_error& err) {
                userConfigTable.reset();
                LOG_ERROR(("Failed to parse user config from: " + userConfigFilepath + ". Ignoring.").c_str(), "Config");
            } catch (const std::exception& e) {
                userConfigTable.reset();
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