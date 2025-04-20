#include "logging/LoggingSystem.hpp"
#include "config/ConfigSystem.hpp"

int main() {
    transity::logging::LoggingSystem::getInstance().initialize();

    LOG_INFO("Core", "Application starting...");

    transity::config::ConfigSystem configSystem;
    configSystem.initialize();

    configSystem.shutdown();
    transity::logging::LoggingSystem::getInstance().shutdown();

    return 0;
}