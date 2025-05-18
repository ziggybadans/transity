#include "Logger.h"
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstdarg>
#include <vector>
#include <cstdio> // For vsnprintf
#include <array>
#include <algorithm> // For std::max (though not used in current priority logic)

namespace Logging {

bool g_isLoggingEnabled = true;
LogLevel g_currentLogLevel = LogLevel::TRACE;
unsigned int g_logDelayMs = 0;
static std::chrono::steady_clock::time_point g_lastLogTime;
std::array<unsigned int, LOG_LEVEL_COUNT> g_logLevelDelays{};

void setLogLevelDelay(LogLevel level, unsigned int delayMs) {
    if (static_cast<int>(level) < LOG_LEVEL_COUNT) {
        g_logLevelDelays[static_cast<int>(level)] = delayMs;
    }
}

unsigned int getLogLevelDelay(LogLevel level) {
    if (static_cast<int>(level) < LOG_LEVEL_COUNT) {
        return g_logLevelDelays[static_cast<int>(level)];
    }
    return 0; // Should not happen with valid LogLevel
}

void setLoggingEnabled(bool enabled) {
    g_isLoggingEnabled = enabled;
}

void setMinLogLevel(LogLevel level) {
    g_currentLogLevel = level;
}

bool isLoggingEnabled() {
    return g_isLoggingEnabled;
}

LogLevel getMinLogLevel() {
    return g_currentLogLevel;
}

void setLogDelay(unsigned int delayMs) {
    g_logDelayMs = delayMs;
}

unsigned int getLogDelay() {
    return g_logDelayMs;
}

const char* logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

void logMessage(LogLevel level, const char* system, unsigned int messageSpecificDelayMs, const char* format, ...) {
    if (!g_isLoggingEnabled || level < g_currentLogLevel) {
        return;
    }

    unsigned int actualDelayToApply = 0;
    if (messageSpecificDelayMs > 0) {
        actualDelayToApply = messageSpecificDelayMs;
    } else {
        unsigned int levelDelay = getLogLevelDelay(level);
        if (levelDelay > 0) {
            actualDelayToApply = levelDelay;
        } else {
            actualDelayToApply = g_logDelayMs;
        }
    }

    if (actualDelayToApply > 0) {
        auto currentTime = std::chrono::steady_clock::now();
        auto timeSinceLastLog = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - g_lastLogTime);
        if (timeSinceLastLog.count() < actualDelayToApply) {
            return;
        }
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;

#ifdef _WIN32
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif

    std::ostringstream timestamp_ss;
    timestamp_ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");

    std::ostringstream message_ss;
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (size >= 0) {
        std::vector<char> buffer(size + 1);
        std::vsnprintf(buffer.data(), buffer.size(), format, args);
        message_ss << buffer.data();
    }
    va_end(args);

    std::cout << timestamp_ss.str() << " [" << system << "] [" << logLevelToString(level) << "] " << message_ss.str() << std::endl;
    g_lastLogTime = std::chrono::steady_clock::now();
}

} // namespace Logging