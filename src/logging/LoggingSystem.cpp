#include "logging/LoggingSystem.h"

#include <iostream>
#include <string>

namespace Transity::Logging {
    void LoggingSystem::addLogger(std::shared_ptr<ILogger> logger) {
        loggers_.push_back(logger);
    }

    void LoggingSystem::log(LogLevel level, const std::string& message) {
        for (const auto& logger : loggers_) {
            logger->log(level, message);
        }
    }
}