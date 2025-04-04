#pragma once

#include "logging/ILogger.h"

namespace Transity::Logging {
    class ConsoleLogger : public ILogger {
        public:
            explicit ConsoleLogger(LogLevel minLevel) : minLogLevel_(minLevel) {}

            void log(LogLevel level, const std::string& message) override;

        private:
            LogLevel minLogLevel_;
    };
}