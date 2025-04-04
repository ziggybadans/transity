#pragma once

#include "logging/ILogger.h"
#include "logging/LogUtils.h"

#include <vector>
#include <memory>

namespace Transity::Logging {
    class LoggingSystem {
        public:
            void addLogger(std::shared_ptr<ILogger> logger);
            void log(LogLevel level, const std::string& message);

        private:
            std::vector<std::shared_ptr<ILogger>> loggers_;
    };
}