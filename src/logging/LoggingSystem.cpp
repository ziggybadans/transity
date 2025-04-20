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

    LOG_DEBUG("Logging", "Initializing sinks...");
    initializeSinks();
    internalLog("Logging system started. Level: INFO. Sinks: Console, File.");
    LOG_DEBUG("Logging", ("Log directory: " + filePath).c_str());
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
/**
 * @brief Initialize configured log sinks
 * @details Creates and activates the requested sink implementations.
 * Skips initialization if test sinks are currently active.
 */
void LoggingSystem::initializeSinks() {
    // --- Variables to store results outside the lock ---
    bool consoleSinkAdded = false;
    bool fileSinkAdded = false;
    std::string initializationErrorMsg;
    std::string finalLogFilePath; // Store the actual path used

    { // --- Start of critical section ---
        std::lock_guard<std::mutex> lock(logMutex);
        if (testingSinksActive) {
            // Still log this immediately if needed, maybe just std::cerr?
            // Or accept that this specific debug log might be lost if testing.
            // For now, let's skip logging inside the lock here.
            return; // Exit early if testing sinks are active
        }
        activeSinks.clear(); // Clear existing sinks

        if (consoleSinkEnabled) {
            activeSinks.push_back(std::make_unique<ConsoleSink>());
            consoleSinkAdded = true; // Mark console sink as added
        }

        if (fileSinkEnabled) {
            try {
                std::filesystem::path target_log_directory(filePath);
                // Attempt to create directories, log error outside if it fails
                std::error_code ec;
                std::filesystem::create_directories(target_log_directory, ec);
                if (ec) {
                    // Store error message instead of logging directly
                    initializationErrorMsg = "Filesystem error creating log directory '" +
                                             target_log_directory.string() + "': " + ec.message();
                    // Don't proceed with file sink creation if directory failed
                } else {
                    // Proceed only if directory creation succeeded or wasn't needed
                    auto now = std::chrono::system_clock::now();
                    auto now_c = std::chrono::system_clock::to_time_t(now);
                    std::tm now_tm = *std::localtime(&now_c); // Note: localtime not thread-safe, consider alternatives if needed later
                    std::ostringstream filename_stream;
                    filename_stream << std::put_time(&now_tm, "%Y-%m-%d_%H-%M-%S");
                    std::string timestamp_str = filename_stream.str();
                    std::string log_filename_only = timestamp_str + ".log";

                    std::filesystem::path full_log_path = target_log_directory / log_filename_only;
                    finalLogFilePath = full_log_path.string(); // Store the path

                    // Create the sink - the constructor might throw std::runtime_error
                    activeSinks.push_back(std::make_unique<FileSink>(finalLogFilePath));
                    fileSinkAdded = true; // Mark file sink as added successfully
                }

            } catch (const std::filesystem::filesystem_error& e) {
                // Store error message instead of logging directly
                initializationErrorMsg = "Filesystem error during log setup: " + std::string(e.what());
                // Consider adding e.path1() and e.path2() if relevant
            } catch (const std::runtime_error& e) {
                // Store error message instead of logging directly
                initializationErrorMsg = "Error creating file log sink: " + std::string(e.what());
            } catch (const std::exception& e) {
                // Catch any other standard exceptions during setup
                initializationErrorMsg = "Unexpected exception during sink initialization: " + std::string(e.what());
            }
        }
    } // --- End of critical section (mutex released here) ---

    // --- Now log the results outside the lock ---
    if (consoleSinkAdded) {
        // Use internalLog directly to bypass level check if needed, or use LOG_DEBUG
        LOG_DEBUG("Logging", "Console sink initialized successfully.");
    }
    if (fileSinkAdded) {
        LOG_DEBUG("Logging", ("File sink initialized successfully for path: " + finalLogFilePath).c_str());
    }

    // Log any error that occurred during initialization
    if (!initializationErrorMsg.empty()) {
        LOG_ERROR("Logging", initializationErrorMsg.c_str());
        throw std::runtime_error("Logging system initialization failed: " + initializationErrorMsg); // Re-throw
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
 * @details Flushes all sinks and releases resources safely.
 */
void LoggingSystem::shutdown() {
    // Log the initial shutdown message (acquires and releases mutex internally)
    internalLog("Logging system shutting down.");

    // Create a temporary copy of sinks to work on outside the main lock
    std::vector<std::unique_ptr<ILogSink>> sinksToShutdown;

    { // --- Start short critical section ---
        std::lock_guard<std::mutex> lock(logMutex);

        // Move ownership of sinks from activeSinks to the temporary vector
        sinksToShutdown = std::move(activeSinks);

        // activeSinks is now empty within the lock
        // No need to call activeSinks.clear() explicitly after std::move

    } // --- End short critical section (mutex released) ---

    // Now, flush and implicitly destroy sinks outside the main lock
    for (auto& sink : sinksToShutdown) {
        try {
            if (sink) { // Check if sink is valid before flushing
                sink->flush();
            }
        } catch (const std::exception& e) {
            // If flush throws, we can only log to stderr now as the logging system is down
            std::cerr << "ERROR: Exception during sink flush on shutdown: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "ERROR: Unknown exception during sink flush on shutdown." << std::endl;
        }
    }
    // Sinks in sinksToShutdown are automatically destroyed when the vector
    // goes out of scope here, closing files etc. via their destructors.

    // Note: This final log message might not make it to the file sink
    // if the file sink was in sinksToShutdown, as it's already flushed and closed.
    // It will likely only go to the console if it was active.
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
    std::cout << message << '\n';
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
