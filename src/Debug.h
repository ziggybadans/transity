#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include "core/ErrorHandler.h"

enum class DebugLevel {
    None,       // No debug output
    Error,      // Only errors
    Warning,    // Errors and warnings
    Info,       // General information
    Debug,      // Detailed debug info
    Verbose     // Everything including detailed tracing
};

class Debug {
public:
    static void SetLevel(DebugLevel level) { m_level = level; }
    static DebugLevel GetLevel() { return m_level; }
    
    template<typename... Args>
    static void Log(DebugLevel level, const Args&... args) {
        if (level <= m_level) {
            std::stringstream ss;
            (ss << ... << args);
            LogMessage(level, ss.str());
        }
    }

    static void EnableFileLogging(const std::string& filename);
    static void DisableFileLogging();

private:
    static void LogMessage(DebugLevel level, const std::string& message);
    static void WriteFormattedMessage(DebugLevel level, const std::string& message);
    static std::string GetLevelString(DebugLevel level);
    static void WriteToFile(const std::string& message);
    static std::string GetTimeString();

    static DebugLevel m_level;
    static std::string m_logFile;
    static bool m_fileLoggingEnabled;
};

// Convenience macros for debug logging
#define DEBUG_ERROR(...) Debug::Log(DebugLevel::Error, __FILE__, ":", __LINE__, " - ", __VA_ARGS__)
#define DEBUG_WARNING(...) Debug::Log(DebugLevel::Warning, __FILE__, ":", __LINE__, " - ", __VA_ARGS__)
#define DEBUG_INFO(...) Debug::Log(DebugLevel::Info, __VA_ARGS__)
#define DEBUG_DEBUG(...) Debug::Log(DebugLevel::Debug, __VA_ARGS__)
#define DEBUG_VERBOSE(...) Debug::Log(DebugLevel::Verbose, __VA_ARGS__) 