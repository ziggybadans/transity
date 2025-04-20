#include "config/ConfigSystem.h"

#include <filesystem>

namespace transity::config {

ConfigSystem::ConfigSystem() {
    initialize();
    LOG_INFO("Config system initialized", "Config");
}

ConfigSystem::~ConfigSystem() {
    LOG_INFO("Config system shutting down", "Config");
    configValues.clear();
}

void ConfigSystem::initialize(const std::string& primaryConfigFilepath) {
    configValues.clear();
    configValues["windowWidth"] = 800;
    configValues["windowHeight"] = 600;
    configValues["windowTitle"] = std::string("Transity");

    LOG_INFO("Default configuration loaded", "Config");

    LOG_INFO(("Attempting to load primary config from: " + primaryConfigFilepath).c_str(), "Config");
}

}