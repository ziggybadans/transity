#include "logging/ConsoleLogger.h"

#include <iostream>

namespace Transity::Logging {
    void ConsoleLogger::log(LogLevel level, const std::string& message) {
        if (level >= minLogLevel_) { std::cout << message << std::endl; }
    }
}