#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
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
    void initialize();
    void initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string filePath = "game_log.log");
    void internalLog(const std::string& message);
    void log(LogLevel level, const char* format, ...);

    LogLevel getLogLevel() const;
    bool isConsoleSinkEnabled() const;
    bool isFileSinkEnabled() const;
    std::string getFilePath() const;

    std::vector<std::unique_ptr<ILogSink>> activeSinks;
protected:
    virtual void initializeSinks();
private:
    std::string levelToString(LogLevel level) const;

    LogLevel logLevel;
    bool consoleSinkEnabled;
    bool fileSinkEnabled;
    std::string filePath;
};

class ConsoleSink : public ILogSink {
public:
    void write(const std::string& message) override;
};

class FileSink : public ILogSink {
public:
    FileSink(const std::string& filePath);
    ~FileSink();
    void write(const std::string& message) override;
private:
    std::string filePath;
    std::ofstream file;
};

}