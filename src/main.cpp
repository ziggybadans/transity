#include "logging/LoggingSystem.h"
#include "logging/ConsoleLogger.h"
#include "logging/FileLogger.h"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[]) {
    auto& logger = Transity::Logging::LoggingSystem::getInstance();
    auto consoleLogger = std::make_shared<Transity::Logging::ConsoleLogger>(Transity::Logging::LogLevel::DEBUG);
    logger.addLogger(consoleLogger);

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    // Note: std::put_time might require #define _CRT_SECURE_NO_WARNINGS on Windows with MSVC
    // or use alternative formatting methods if needed.
    ss << "logs/app_log_" << std::put_time(std::localtime(&now_c), "%Y%m%d_%H%M%S") << ".log";
    std::string uniqueLogFilename = ss.str();

    std::filesystem::path logDirectory = "logs";

    try {
        if (!logDirectory.empty() && !std::filesystem::exists(logDirectory)) {
            if (std::filesystem::create_directories(logDirectory)) {
                std::cout << "Log directory created successfully." << std::endl;
            } else {
                std::cerr << "Failed to create log directory." << std::endl;
            }
        }    

        auto fileLogger = std::make_shared<Transity::Logging::FileLogger>(uniqueLogFilename);

        logger.addLogger(fileLogger);
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle the error - maybe log to console if possible, or exit?
        std::cerr << "Error creating log directory: " << e.what() << std::endl;
        // Depending on severity, you might exit(1);
    }
    
    logger.log(Transity::Logging::LogLevel::INFO, "Hello, world!");

    logger.log(Transity::Logging::LogLevel::INFO, "Application shutting down.");
    return 0;
}