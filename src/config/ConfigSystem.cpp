#include "config/ConfigSystem.h"

namespace transity::config {

ConfigSystem::ConfigSystem() {
    initialize();
    LOG_INFO("Config system initialized", "Config");
}

ConfigSystem::~ConfigSystem() {
    LOG_INFO("Config system shutting down", "Config");
    configValues.clear();
}

void ConfigSystem::initialize() {
    configValues["windowWidth"] = 800;
    configValues["windowHeight"] = 600;
    configValues["windowTitle"] = std::string("Transity");

    LOG_INFO("Default configuration loaded", "Config");
}

}