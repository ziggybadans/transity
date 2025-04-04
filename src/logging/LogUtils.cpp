#include "logging/LogUtils.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Transity::Logging {
    std::string logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }

    std::string formatLogMessage(LogLevel level, const std::string& message) {
        // 1. Get current time
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);

        // ! Not thread safe
        std::tm now_tm = *std::localtime(&now_c);

        // 2. Format time
        std::stringstream timeStream;
        timeStream << std::put_time(&now_tm, "[%Y-%m-%d %H:%M:%S]");

        // 3. Get level string
        std::string levelStr = logLevelToString(level);

        // 4. Construct message
        std::stringstream formattedMessage;
        formattedMessage << timeStream.str() << " [" << levelStr << "] " << message;

        return formattedMessage.str();
    }
}