#pragma once

#include <string>

namespace Transity::Logging {
    enum class LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    std::string logLevelToString(LogLevel level);
}