#include "logging/FileLogger.h"
#include "logging/LogUtils.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Transity::Logging {
    FileLogger::FileLogger(const std::string& filename) : 
        filename_(filename), 
        file_(filename_, std::ios::app) {
            if (!file_.is_open()) {
                std::cerr << "Error: Failed to open log file in constructor: " << filename_ << std::endl;
        }
    }

    void FileLogger::log(LogLevel level, const std::string& message) {
        if (file_.is_open()) {
            file_ << formatLogMessage(level, message) << std::endl;
        } else {
            std::cerr << "Failed to open log file: " << filename_ << std::endl;
        }
    }
}