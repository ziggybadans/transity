/**
 * @file logging_system.h
 * @brief Logging system interface for the Transity engine
 *
 * Provides a thread-safe logging system with multiple log levels and output sinks.
 * Supports both console and file output, with configurable log levels.
 */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <mutex>
#include "logging/ILogSink.h"

namespace transity::logging {

/**
 * @enum LogLevel
 * @brief Defines the severity levels for log messages
 */
enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @class LoggingSystem
 * @brief Singleton class providing logging functionality
 *
 * Thread-safe logging system supporting multiple output sinks and log levels.
 * Use the LOG_* macros for convenient logging throughout the application.
 */
class LoggingSystem {
public:
    LoggingSystem(const LoggingSystem&) = delete;
    LoggingSystem& operator=(const LoggingSystem&) = delete;

    /**
     * @brief Get the singleton instance of the logging system
     * @return Reference to the LoggingSystem instance
     */
    static LoggingSystem& getInstance() {
        static LoggingSystem instance;
        return instance;
    }

    /**
     * @brief Initialize logging system with default settings
     * @details Defaults to INFO level with both console and file sinks enabled
     */
    void initialize();
    /**
     * @brief Initialize logging system with custom settings
     * @param level Minimum log level to output
     * @param enableFileSink Whether to enable file output
     * @param enableConsoleSink Whether to enable console output
     * @param filePath Path to log file (default: "game_log.log")
     */
    void initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string filePath = "game_log.log");
    /**
     * @brief Log a formatted message
     * @param level Severity level of the message
     * @param system Name of the system/component generating the log
     * @param format printf-style format string
     * @param ... Variable arguments for the format string
     */
    void log(LogLevel level, const char* system, const char* format, ...);
    /**
     * @brief Shutdown the logging system
     * @details Flushes all sinks and releases resources
     */
    void shutdown();
    /**
     * @brief Replace active sinks with test sinks
     * @param sinks Vector of test sink implementations
     * @note For testing purposes only
     */
    void setSinksForTesting(std::vector<std::unique_ptr<ILogSink>> sinks);

    /**
     * @brief Get current minimum log level
     * @return Current LogLevel setting
     */
    LogLevel getLogLevel() const;
    /**
     * @brief Check if console output is enabled
     * @return true if console output is enabled
     */
    bool isConsoleSinkEnabled() const;
    /**
     * @brief Check if file output is enabled
     * @return true if file output is enabled
     */
    bool isFileSinkEnabled() const;
    /**
     * @brief Get current log file path
     * @return Path to current log file
     */
    std::string getFilePath() const;

    /// Currently active log sinks (public for testing purposes)
    std::vector<std::unique_ptr<ILogSink>> activeSinks;
protected:
    /**
     * @brief Initialize configured sinks
     * @details Called during system initialization
     */
    virtual void initializeSinks();
    /**
     * @brief Internal method to write message to all sinks
     * @param message Formatted log message to output
     */
    void internalLog(const std::string& message);
    /**
     * @brief Reset logging system to default state
     * @details For testing purposes
     */
    void resetToDefaults();
private:
    LoggingSystem() = default;
    /**
     * @brief Convert LogLevel to string representation
     * @param level LogLevel to convert
     * @return String representation of the level
     */
    std::string levelToString(LogLevel level) const;

    LogLevel logLevel;
    bool consoleSinkEnabled;
    bool fileSinkEnabled;
    std::string filePath;
    std::mutex logMutex;
    bool testingSinksActive = false;
};

/**
 * @class ConsoleSink
 * @brief Log sink implementation for console output
 */
class ConsoleSink : public ILogSink {
public:
    void write(const std::string& message) override;
    void flush() override;
};

/**
 * @class FileSink
 * @brief Log sink implementation for file output
 */
class FileSink : public ILogSink {
public:
    FileSink(const std::string& filePath);
    ~FileSink();
    void write(const std::string& message) override;
    void flush() override;
private:
    std::string filePath;
    std::ofstream file;
};

/**
 * @def LOG_TRACE(system, format, ...)
 * @brief Log a TRACE level message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
#define LOG_TRACE(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::TRACE, system, format, ##__VA_ARGS__)
/**
 * @def LOG_DEBUG(system, format, ...)
 * @brief Log a DEBUG level message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
#define LOG_DEBUG(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::DEBUG, system, format, ##__VA_ARGS__)
/**
 * @def LOG_INFO(system, format, ...)
 * @brief Log an INFO level message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
#define LOG_INFO(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::INFO, system, format, ##__VA_ARGS__)
/**
 * @def LOG_WARN(system, format, ...)
 * @brief Log a WARN level message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
#define LOG_WARN(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::WARN, system, format, ##__VA_ARGS__)
/**
 * @def LOG_ERROR(system, format, ...)
 * @brief Log an ERROR level message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
#define LOG_ERROR(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::ERROR, system, format, ##__VA_ARGS__)
/**
 * @def LOG_FATAL(system, format, ...)
 * @brief Log a FATAL level message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
#define LOG_FATAL(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::FATAL, system, format, ##__VA_ARGS__)

}