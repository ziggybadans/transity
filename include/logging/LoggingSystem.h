#pragma once

#include "logging/ILogger.h"
#include "logging/LogUtils.h"

#include <vector>
#include <memory>

namespace Transity::Logging {
    class LoggingSystem {
        public:
            static LoggingSystem& getInstance();

            void addLogger(std::shared_ptr<ILogger> logger);
            void log(LogLevel level, const std::string& message);

            #ifdef ENABLE_TESTING
                static void reset();
            #endif

        private:
            // Make constructor private/deleted for singleton
            LoggingSystem() = default;
            ~LoggingSystem() = default;
            LoggingSystem(const LoggingSystem&) = delete;
            LoggingSystem& operator=(const LoggingSystem&) = delete;
            LoggingSystem(LoggingSystem&&) = delete;
            LoggingSystem& operator=(LoggingSystem&&) = delete;

            std::vector<std::shared_ptr<ILogger>> loggers_;
    };
}