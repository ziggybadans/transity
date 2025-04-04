#include "logging/ConsoleLogger.h"
#include "logging/LogUtils.h"

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Transity::Logging {
    void ConsoleLogger::log(LogLevel level, const std::string& message) {
        // Check filtering level
        if (level >= minLogLevel_) {
            // Print to appropriate stream
            if (level >= LogLevel::ERROR) {
                std::cerr << formatLogMessage(level, message) << std::endl;
            } else {
                std::cout << formatLogMessage(level, message) << std::endl;
            }
        }
    }
}