#include "logging/LoggingSystem.hpp"

int main() {
    transity::logging::LoggingSystem::getInstance().initialize();

    LOG_INFO("Core", "Application starting...");

    transity::logging::LoggingSystem::getInstance().shutdown();

    return 0;
}