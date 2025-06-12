#pragma once

#include <string>
#include <cstdarg>
#include <utility>
#include <array>
#include <fstream>
#include <chrono>

namespace Logging {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

constexpr int LOG_LEVEL_COUNT = static_cast<int>(LogLevel::FATAL) + 1;

class Logger {
public:
    static Logger& getInstance();

    ~Logger();

    void setLoggingEnabled(bool enabled);
    void setMinLogLevel(LogLevel level);
    bool isLoggingEnabled() const;
    LogLevel getMinLogLevel() const;
    void setLogDelay(unsigned int delayMs);
    unsigned int getLogDelay() const;
    const char* logLevelToString(LogLevel level) const;
    void setLogLevelDelay(LogLevel level, unsigned int delayMs);
    unsigned int getLogLevelDelay(LogLevel level) const;

    void logMessage(LogLevel level, const char* system, unsigned int messageSpecificDelayMs, const char* format, ...);

    void enableFileLogging(bool enable);

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    bool m_isLoggingEnabled = true;
    LogLevel m_currentLogLevel = LogLevel::TRACE;
    unsigned int m_logDelayMs = 0;
    std::chrono::steady_clock::time_point m_lastLogTime;
    std::array<unsigned int, LOG_LEVEL_COUNT> m_logLevelDelays{};

    bool m_isFileLoggingEnabled = false;
    std::ofstream m_logFileStream;
    std::string m_logDirectory = "logs";
    std::string m_currentLogFileName;
};

}

namespace LoggerMacrosImpl {

template<typename... Args>
inline void log_trace_proxy(const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::TRACE, system, 0, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_trace_proxy(unsigned int delayMs, const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::TRACE, system, delayMs, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_debug_proxy(const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::DEBUG, system, 0, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_debug_proxy(unsigned int delayMs, const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::DEBUG, system, delayMs, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_info_proxy(const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::INFO, system, 0, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_info_proxy(unsigned int delayMs, const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::INFO, system, delayMs, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_warn_proxy(const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::WARN, system, 0, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_warn_proxy(unsigned int delayMs, const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::WARN, system, delayMs, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_error_proxy(const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::ERROR, system, 0, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_error_proxy(unsigned int delayMs, const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::ERROR, system, delayMs, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_fatal_proxy(const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::FATAL, system, 0, format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log_fatal_proxy(unsigned int delayMs, const char* system, const char* format, Args&&... args) {
    Logging::Logger::getInstance().logMessage(Logging::LogLevel::FATAL, system, delayMs, format, std::forward<Args>(args)...);
}

}

#define LOG_TRACE(...) LoggerMacrosImpl::log_trace_proxy(__VA_ARGS__)
#define LOG_DEBUG(...) LoggerMacrosImpl::log_debug_proxy(__VA_ARGS__)
#define LOG_INFO(...)  LoggerMacrosImpl::log_info_proxy(__VA_ARGS__)
#define LOG_WARN(...)  LoggerMacrosImpl::log_warn_proxy(__VA_ARGS__)
#define LOG_ERROR(...) LoggerMacrosImpl::log_error_proxy(__VA_ARGS__)
#define LOG_FATAL(...) LoggerMacrosImpl::log_fatal_proxy(__VA_ARGS__)