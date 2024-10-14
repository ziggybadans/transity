// src/utils/Logger.cpp
#include "Logger.h"
#include <iostream>

// Open the log file in append mode to preserve previous logs
std::ofstream Logger::logFile("game.log", std::ios::app);

// Logs a message to both the log file and the console
void Logger::log(const std::string& message) {
    if (logFile.is_open()) { // If the log file is open, write the message to it
        logFile << message << std::endl;
    }
    std::cout << message << std::endl; // Print the message to the console
}

// Summary:
// The Logger class provides a simple way to log messages to both a file ("game.log") and the console.
// The log function checks if the file is open and writes the log message to it, as well as printing
// the same message to the standard output. This dual logging can be useful for debugging purposes.