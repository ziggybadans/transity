#pragma once

#include "logging/ILogger.h"
#include "logging/LogUtils.h"

#include <fstream>

namespace Transity::Logging {
    class FileLogger : public ILogger {
        public:
            explicit FileLogger(const std::string& filename);

            void log(LogLevel level, const std::string& message) override;

        private:
            std::string filename_;
            std::ofstream file_;
    };
}