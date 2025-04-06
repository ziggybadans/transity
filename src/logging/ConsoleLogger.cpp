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
        // 1. Check filtering level
        if (level >= minLogLevel_) {
            // 2. Get current time
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);

            // ! Not thread safe
            std::tm now_tm = *std::localtime(&now_c);

            // 3. Format time
            std::stringstream timeStream;
            timeStream << std::put_time(&now_tm, "[%Y-%m-%d %H:%M:%S]");

            // 4. Get level string
            std::string levelStr = logLevelToString(level);

            // 5. Construct message
            std::stringstream formattedMessage;
            formattedMessage << timeStream.str() << " [" << levelStr << "] " << message;

            // 6. Print to appropriate stream
            if (level >= LogLevel::ERROR) {
                std::cerr << formattedMessage.str() << std::endl;
            } else {
                std::cout << formattedMessage.str() << std::endl;
            }
        }
    }
}