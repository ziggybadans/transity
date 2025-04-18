#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <mutex>
#include "logging/ILogSink.h"

namespace transity::logging {

enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class LoggingSystem {
public:
    LoggingSystem(const LoggingSystem&) = delete;
    LoggingSystem& operator=(const LoggingSystem&) = delete;

    static LoggingSystem& getInstance() {
        static LoggingSystem instance;
        return instance;
    }

    void initialize();
    void initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string filePath = "game_log.log");
    void log(LogLevel level, const char* system, const char* format, ...);
    void shutdown();
    void setSinksForTesting(std::vector<std::unique_ptr<ILogSink>> sinks);

    LogLevel getLogLevel() const;
    bool isConsoleSinkEnabled() const;
    bool isFileSinkEnabled() const;
    std::string getFilePath() const;

    std::vector<std::unique_ptr<ILogSink>> activeSinks;
protected:
    virtual void initializeSinks();
    void internalLog(const std::string& message);
    void resetToDefaults();
private:
    LoggingSystem() = default;
    std::string levelToString(LogLevel level) const;

    LogLevel logLevel;
    bool consoleSinkEnabled;
    bool fileSinkEnabled;
    std::string filePath;
    std::mutex logMutex;
    bool testingSinksActive = false;
};

class ConsoleSink : public ILogSink {
public:
    void write(const std::string& message) override;
    void flush() override;
};

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

#define LOG_TRACE(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::TRACE, system, format, ##__VA_ARGS__)
#define LOG_DEBUG(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::DEBUG, system, format, ##__VA_ARGS__)
#define LOG_INFO(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::INFO, system, format, ##__VA_ARGS__)
#define LOG_WARN(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::WARN, system, format, ##__VA_ARGS__)
#define LOG_ERROR(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::ERROR, system, format, ##__VA_ARGS__)
#define LOG_FATAL(system, format, ...) transity::logging::LoggingSystem::getInstance().log(transity::logging::LogLevel::FATAL, system, format, ##__VA_ARGS__)

}