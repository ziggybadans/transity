#pragma once

#include <string>
#include "logging/LogUtils.h"

namespace Transity::Logging {

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
};

}