#pragma once

#include <string>

namespace Transity {
namespace Logging {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void write(const std::string& message) = 0;
};
}
}