#include "Logger.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Logging {

Logger* g_logger = nullptr;

Logger::Logger() : m_lastLogTime(std::chrono::steady_clock::now()) {}

Logger::~Logger() {
    if (m_logFileStream.is_open()) {
        m_logFileStream.close();
    }
}

void Logger::setLogLevelDelay(LogLevel level, unsigned int delayMs) {
    if (static_cast<int>(level) < LOG_LEVEL_COUNT) {
        m_logLevelDelays[static_cast<int>(level)] = delayMs;
    }
}

unsigned int Logger::getLogLevelDelay(LogLevel level) const {
    if (static_cast<int>(level) < LOG_LEVEL_COUNT) {
        return m_logLevelDelays[static_cast<int>(level)];
    }
    return 0;
}

void Logger::setLoggingEnabled(bool enabled) {
    m_isLoggingEnabled = enabled;
}

void Logger::setMinLogLevel(LogLevel level) {
    m_currentLogLevel = level;
}

bool Logger::isLoggingEnabled() const {
    return m_isLoggingEnabled;
}

LogLevel Logger::getMinLogLevel() const {
    return m_currentLogLevel;
}

void Logger::setLogDelay(unsigned int delayMs) {
    m_logDelayMs = delayMs;
}

unsigned int Logger::getLogDelay() const {
    return m_logDelayMs;
}

const char *Logger::logLevelToString(LogLevel level) const {
    switch (level) {
    case LogLevel::TRACE:
        return "TRACE";
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

void Logger::enableFileLogging(bool enable) {
    if (enable && !m_isFileLoggingEnabled) {
        try {
            if (!std::filesystem::exists(m_logDirectory)) {
                std::filesystem::create_directory(m_logDirectory);
            }

            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm buf;
#ifdef _WIN32
            localtime_s(&buf, &in_time_t);
#else
            localtime_r(&in_time_t, &buf);
#endif
            std::ostringstream filename_ss;
            filename_ss << m_logDirectory << "/" << std::put_time(&buf, "%Y-%m-%d_%H-%M-%S")
                        << ".log";
            m_currentLogFileName = filename_ss.str();
            m_logFileStream.open(m_currentLogFileName, std::ios::out | std::ios::app);
            if (m_logFileStream.is_open()) {
                m_isFileLoggingEnabled = true;
            } else {
                std::cerr << "Error: Could not open log file: " << m_currentLogFileName
                          << std::endl;
                m_isFileLoggingEnabled = false;
            }
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            m_isFileLoggingEnabled = false;
        }
    } else if (!enable && m_isFileLoggingEnabled) {
        if (m_logFileStream.is_open()) {
            m_logFileStream.close();
        }
        m_isFileLoggingEnabled = false;
        m_currentLogFileName.clear();
    }
}

static const int SYSTEM_NAME_WIDTH = 13;
static const int LOG_LEVEL_WIDTH = 7;

void Logger::logMessage(LogLevel level, const char *system, unsigned int messageSpecificDelayMs,
                        const char *format, ...) {
    if (!m_isLoggingEnabled || level < m_currentLogLevel) {
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
            actualDelayToApply = m_logDelayMs;
        }
    }

    if (actualDelayToApply > 0) {
        auto currentTime = std::chrono::steady_clock::now();
        auto timeSinceLastLog =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastLogTime);
        if (timeSinceLastLog.count() < actualDelayToApply) {
            return;
        }
    }

    auto now_sys = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now_sys);
    std::tm buf;

#ifdef _WIN32
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif

    std::ostringstream timestamp_ss;
    timestamp_ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    std::string timestamp_str = timestamp_ss.str();

    std::string system_name_for_padding(system);
    if (system_name_for_padding.length() > SYSTEM_NAME_WIDTH) {
        system_name_for_padding.resize(SYSTEM_NAME_WIDTH);
    }
    std::string log_level_for_padding(logLevelToString(level));
    if (log_level_for_padding.length() > LOG_LEVEL_WIDTH) {
        log_level_for_padding.resize(LOG_LEVEL_WIDTH);
    }

    std::ostringstream formatted_log_prefix_ss;
    formatted_log_prefix_ss << timestamp_str << " [" << std::left << std::setw(SYSTEM_NAME_WIDTH)
                            << system_name_for_padding << "] [" << std::left
                            << std::setw(LOG_LEVEL_WIDTH) << log_level_for_padding << "] ";
    std::string formatted_log_prefix = formatted_log_prefix_ss.str();

    std::ostringstream message_ss;
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    std::string formatted_message_str;
    if (size >= 0) {
        std::vector<char> buffer(size + 1);
        std::vsnprintf(buffer.data(), buffer.size(), format, args);
        formatted_message_str = buffer.data();
    }
    va_end(args);

    std::cout << formatted_log_prefix << formatted_message_str << std::endl;

    if (m_isFileLoggingEnabled && m_logFileStream.is_open()) {
        m_logFileStream << formatted_log_prefix << formatted_message_str << std::endl;
    }

    m_lastLogTime = std::chrono::steady_clock::now();
}

}  // namespace Logging