#include "logging/logging_system.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <vector>
#include <string>

namespace transity::logging {

// Default config
void LoggingSystem::initialize() {
    logLevel = LogLevel::INFO;
    consoleSinkEnabled = true;
    fileSinkEnabled = true;
    filePath = "game_log.log";

    initializeSinks();
    internalLog("Logging system started. Level: INFO. Sinks: Console, File.");
}

// Overloaded method for custom config
void LoggingSystem::initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string path) {
    logLevel = level;
    consoleSinkEnabled = enableConsoleSink;
    fileSinkEnabled = enableFileSink;
    filePath = path;

    if (fileSinkEnabled) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filePath);
        }
    }

    initializeSinks();
    internalLog("Logging system started. Level: " + std::to_string(level) + ". Sinks: " + (enableConsoleSink ? "Console, " : "") + (enableFileSink ? "File." : ""));
}

void LoggingSystem::initializeSinks() {
    activeSinks.clear();
    if (consoleSinkEnabled) {
        activeSinks.push_back(std::make_unique<ConsoleSink>());
    }
    if (fileSinkEnabled) {
        activeSinks.push_back(std::make_unique<FileSink>(filePath));
    }
}

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

void LoggingSystem::log(LogLevel level, const char* format, ...) {
    if (level < logLevel) {
        return;
    }

    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int requiredSize = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    std::string formatted_message_part;
    if (requiredSize > 0) {
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

    std::ostringstream oss_formatted;
    oss_formatted << std::left // Align text to the left within the width
                << std::setw(24) << (dateTimeStr + "." + msStr) // Timestamp column (adjust width 24)
                << std::setw(8) << ("[" + levelToString(level) + "]") // Level column (adjust width 8)
                //<< std::setw(10) << "[TID: " << threadIdStr << "]"
                << ""
                << formatted_message_part; // The rest of the message
    std::string formattedMessage = oss_formatted.str();
    internalLog(formattedMessage);
}

void LoggingSystem::internalLog(const std::string& message) {
    for (auto& sink : activeSinks) {
        sink->write(message);
    }
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

void ConsoleSink::write(const std::string& message) {
    std::cout << message << std::endl;
}

FileSink::FileSink(const std::string& filePath) : filePath(filePath) {
    file = std::ofstream(filePath, std::ios_base::app);
}

FileSink::~FileSink() {
    if (file.is_open()) {
        file.close();
    }
}

void FileSink::write(const std::string& message) {
    if (file.is_open()) {
        file << message << std::endl;
    }
}

}