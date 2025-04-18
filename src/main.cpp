#include "logging/logging_system.h"

int main() {
    transity::logging::LoggingSystem::getInstance().initialize();

    LOG_INFO("Core", "Application starting...");

    transity::logging::LoggingSystem::getInstance().shutdown();

    return 0;
}