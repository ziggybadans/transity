#include "logging/LoggingSystem.h"

#include <iostream>
#include <string>

namespace Transity::Logging {
    LoggingSystem& LoggingSystem::getInstance() {
        static LoggingSystem instance; // Creates the single instance on first call
        return instance;
    }

    #ifdef ENABLE_TESTING // Match the guard in the header
    void LoggingSystem::reset() {
        // Get the single instance
        LoggingSystem& instance = getInstance();
        // Clear its state
        instance.loggers_.clear();
        // Reset any other state if added later (e.g., minLogLevel_)
    }
    #endif

    void LoggingSystem::addLogger(std::shared_ptr<ILogger> logger) {
        if (logger) {
            loggers_.push_back(logger);
        }
    }

    void LoggingSystem::log(LogLevel level, const std::string& message) {
        for (const auto& logger : loggers_) {
            logger->log(level, message);
        }
    }
}