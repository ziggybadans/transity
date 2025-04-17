#pragma once
#include <string>

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
    void initialize(LogLevel level, bool enableFileSink, bool enableConsoleSink, std::string filePath = "game_log.txt");
    LogLevel getLogLevel() const;
    bool isConsoleSinkEnabled() const;
    bool isFileSinkEnabled() const;
    std::string getFilePath() const;
private:
    LogLevel logLevel;
    bool consoleSinkEnabled;
    bool fileSinkEnabled;
    std::string filePath;
};

}