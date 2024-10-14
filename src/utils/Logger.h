// src/utils/Logger.h
#pragma once
#include <string>    // Include for handling log messages
#include <fstream>   // Include for file stream operations

class Logger {
public:
    // Logs a message to both the log file and the console
    static void log(const std::string& message);

private:
    static std::ofstream logFile; // File stream for logging messages to a file
};

// Summary:
// The Logger class is a utility class that provides a static method for logging messages to both a file and the console.
// It uses an ofstream to write messages to "game.log" and ensures all messages are also printed to the console.
// This approach helps in keeping a persistent log while allowing real-time monitoring of the application's state.