/**
 * @file LoggingSystem.cpp
 * @brief Implementation of the Transity engine logging system
 *
 * Contains the concrete implementation of all logging system functionality
 * declared in LoggingSystem.h.
 */
#include "logging/LoggingSystem.hpp"

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <vector>
#include <string>
#include <thread>
#include <regex>
#include <filesystem>

namespace transity::logging {

/**
 * @brief Initialize logging system with default configuration
 *
 * Sets up logging with:
 * - Log level: INFO
 * - Both console and file sinks enabled
 * - Default log file path: "game_log.log"
 */
void LoggingSystem::initialize() {
    logLevel = LogLevel::INFO;
    consoleSinkEnabled = true;
    fileSinkEnabled = true;
    filePath = "logs";

    initializeSinks();
    internalLog("Logging system started. Level: INFO. Sinks: Console, File.");
}

/**
 * @brief Initialize logging system with custom configuration
 * @param level Minimum log level to output
 * @param enableFileSink Whether to enable file output
 * @param enableConsoleSink Whether to enable console output
 * @param path Path to log file (default: "game_log.log")
 */
void LoggingSystem::initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string path) {
    logLevel = level;
    consoleSinkEnabled = enableConsoleSink;
    fileSinkEnabled = enableFileSink;
    filePath = path;

    initializeSinks();
    internalLog("Logging system started. Level: " + std::to_string(level) + ". Sinks: " + (enableConsoleSink ? "Console, " : "") + (enableFileSink ? "File." : ""));
}

/**
 * @brief Initialize configured log sinks
 * @details Creates and activates the requested sink implementations.
 * Skips initialization if test sinks are currently active.
 */
void LoggingSystem::initializeSinks() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (testingSinksActive) {
        return;
    }
    activeSinks.clear();

    if (consoleSinkEnabled) {
        activeSinks.push_back(std::make_unique<ConsoleSink>());
    }
    if (fileSinkEnabled) {
        try {
            std::filesystem::path target_log_directory(filePath);
            std::filesystem::create_directories(target_log_directory);

            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_c);
            std::ostringstream filename_stream;
            filename_stream << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S");
            std::string timestamp_str = filename_stream.str();
            std::string log_filename_only = timestamp_str + ".log";

            std::filesystem::path full_log_path = target_log_directory / log_filename_only;
            activeSinks.push_back(std::make_unique<FileSink>(full_log_path.string()));
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error during log setup: " << e.what() << std::endl;
            throw;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error creating file log sink: " << e.what() << std::endl;
            throw;
        }
    }
}

/**
 * @brief Convert LogLevel enum to human-readable string
 * @param level LogLevel to convert
 * @return String representation of the log level
 */
std::string LoggingSystem::levelToString(LogLevel level) const {
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

/**
 * @brief Log a formatted message
 * @details Handles:
 * - Log level filtering
 * - Thread-safe message formatting
 * - Timestamp generation
 * - Thread ID capture
 * - Message formatting with variable arguments
 * - Output to all active sinks
 *
 * @param level Severity level of the message
 * @param system Name of the system/component generating the log
 * @param format printf-style format string
 * @param ... Variable arguments for the format string
 */
void LoggingSystem::log(LogLevel level, const char* system, const char* format, ...) {
    if (level < logLevel) {
        return;
    }

    std::thread::id this_id = std::this_thread::get_id();
    std::ostringstream oss_tid;
    oss_tid << this_id;
    std::string threadIdStr = oss_tid.str();

    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int requiredSize = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    std::string formatted_message_part;
    if (requiredSize >= 0) {
        std::vector<char> buffer(requiredSize + 1);
        std::vsnprintf(buffer.data(), buffer.size(), format, args);
        formatted_message_part = std::string(buffer.data());
    } else {
        formatted_message_part = "Error formatting log message";
    }

    va_end(args);

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    std::ostringstream oss_dt;
    oss_dt << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    std::string dateTimeStr = oss_dt.str();

    auto timeSinceEpoch = now.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch);
    long long milliseconds = ms.count() % 1000;

    std::ostringstream oss_ms;
    oss_ms << std::setw(3) << std::setfill('0') << milliseconds;
    std::string msStr = oss_ms.str();

    std::string timestamp_field = dateTimeStr + "." + msStr;
    std::string tid_field = "[TID: " + threadIdStr + "]";
    std::string level_field = "[" + levelToString(level) + "]";
    std::string system_field = "[" + std::string(system) + "]";

    std::ostringstream oss_formatted;
    oss_formatted << std::left // Apply left alignment once
                  << std::setw(24) << timestamp_field // Timestamp
                  << " " // Separator
                  << std::setw(12) << tid_field       // Thread ID (adjust width 12 or as needed)
                  << " " // Separator
                  << std::setw(8) << level_field       // Level
                  << " " // Separator
                  << std::setw(10) << system_field      // System (adjust width 10 or as needed)
                  << " " // Separator
                  << formatted_message_part;          // The actual message
    std::string formattedMessage = oss_formatted.str();
    internalLog(formattedMessage);
}

/**
 * @brief Internal method to write message to all active sinks
 * @details Thread-safe operation that writes to all configured sinks
 * @param message Formatted log message to output
 */
void LoggingSystem::internalLog(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    for (auto& sink : activeSinks) {
        sink->write(message);
    }
}

/**
 * @brief Shutdown the logging system
 * @details Flushes all sinks and releases resources
 */
void LoggingSystem::shutdown() {
    internalLog("Logging system shutting down.");
    std::lock_guard<std::mutex> lock(logMutex);
    for (auto& sink : activeSinks) {
        sink->flush();
    }
    activeSinks.clear();
}

/**
 * @brief Replace active sinks with test sinks
 * @param sinks Vector of test sink implementations
 * @note For testing purposes only
 */
void LoggingSystem::setSinksForTesting(std::vector<std::unique_ptr<ILogSink>> sinks) {
    std::lock_guard<std::mutex> lock(logMutex);
    activeSinks = std::move(sinks);
    testingSinksActive = true;
}

/**
 * @brief Reset logging system to default state
 * @details Clears test sinks and resets to normal operation
 * @note For testing purposes only
 */
void LoggingSystem::resetToDefaults() {
    std::lock_guard<std::mutex> lock(logMutex);
    activeSinks.clear();
    testingSinksActive = false;
}

LogLevel LoggingSystem::getLogLevel() const {
    return logLevel;
}

bool LoggingSystem::isConsoleSinkEnabled() const {
    return consoleSinkEnabled;
}

bool LoggingSystem::isFileSinkEnabled() const {
    return fileSinkEnabled;
}

std::string LoggingSystem::getFilePath() const {
    return filePath;
}

/**
 * @brief Write message to console
 * @param message Formatted log message to output
 */
void ConsoleSink::write(const std::string& message) {
    std::cout << message << std::endl;
}

/**
 * @brief Flush console output
 */
void ConsoleSink::flush() {
    std::cout.flush();
}

/**
 * @brief Construct FileSink with specified log file path
 * @param filePath Path to log file
 * @throws std::runtime_error if file cannot be opened
 */
FileSink::FileSink(const std::string& filePath) : filePath(filePath) {
    file = std::ofstream(filePath, std::ios_base::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filePath);
    }
}

/**
 * @brief FileSink destructor
 * @details Ensures log file is properly closed
 */
FileSink::~FileSink() {
    if (file.is_open()) {
        file.close();
    }
}

/**
 * @brief Write message to log file
 * @param message Formatted log message to output
 */
void FileSink::write(const std::string& message) {
    if (file.is_open()) {
        file << message << std::endl;
    }
}

/**
 * @brief Flush file output
 */
void FileSink::flush() {
    if (file.is_open()) {
        file.flush();
    }
}

}