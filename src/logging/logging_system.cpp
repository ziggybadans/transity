#include "logging/logging_system.h"

namespace transity::logging {

// Default config
void LoggingSystem::initialize() {
    logLevel = LogLevel::INFO;
    consoleSinkEnabled = true;
    fileSinkEnabled = false;
    filePath = "game_log.txt";
}

// Overloaded method for custom config
void LoggingSystem::initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string path) {
    logLevel = level;
    consoleSinkEnabled = enableConsoleSink;
    fileSinkEnabled = enableFileSink;
    filePath = path;
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

}