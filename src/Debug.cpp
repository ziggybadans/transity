#include "Debug.h"
#include <fstream>
#include <chrono>
#include <iomanip>

DebugLevel Debug::m_level = DebugLevel::Warning;
std::string Debug::m_logFile;
bool Debug::m_fileLoggingEnabled = false;

void Debug::EnableFileLogging(const std::string& filename) {
    m_logFile = filename;
    m_fileLoggingEnabled = true;
}

void Debug::DisableFileLogging() {
    m_fileLoggingEnabled = false;
}

std::string Debug::GetTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &time);
#else
    localtime_r(&time, &timeInfo);
#endif

    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Debug::LogMessage(DebugLevel level, const std::string& message) {
    std::stringstream ss;
    ss << "[" << GetTimeString() << "] "
       << "[" << GetLevelString(level) << "] " 
       << message;

    // Output to console
    if (level == DebugLevel::Error) {
        std::cerr << ss.str() << std::endl;
        ErrorHandler::ReportError(ErrorSeverity::Error, message);
    } else {
        std::cout << ss.str() << std::endl;
    }

    // Write to file if enabled
    if (m_fileLoggingEnabled) {
        WriteToFile(ss.str());
    }
}

std::string Debug::GetLevelString(DebugLevel level) {
    switch (level) {
        case DebugLevel::Error: return "ERROR";
        case DebugLevel::Warning: return "WARNING";
        case DebugLevel::Info: return "INFO";
        case DebugLevel::Debug: return "DEBUG";
        case DebugLevel::Verbose: return "VERBOSE";
        default: return "NONE";
    }
}

void Debug::WriteToFile(const std::string& message) {
    try {
        std::ofstream file(m_logFile, std::ios::app);
        if (file.is_open()) {
            file << message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to write to log file: " << e.what() << std::endl;
    }
}

void Debug::WriteFormattedMessage(DebugLevel level, const std::string& message) {
    std::stringstream ss;
    ss << "[" << GetTimeString() << "] "
       << "[" << GetLevelString(level) << "] " 
       << message;

    // Output to console based on severity
    if (level == DebugLevel::Error) {
        std::cerr << ss.str() << std::endl;
        ErrorHandler::ReportError(ErrorSeverity::Error, message);
    } else if (level == DebugLevel::Warning) {
        std::cout << ss.str() << std::endl;
        ErrorHandler::ReportError(ErrorSeverity::Warning, message);
    } else if (level == DebugLevel::Info) {
        std::cout << ss.str() << std::endl;
        ErrorHandler::ReportError(ErrorSeverity::Info, message);
    } else {
        std::cout << ss.str() << std::endl;
    }

    // Write to log file if enabled
    if (m_fileLoggingEnabled) {
        WriteToFile(ss.str());
    }
} 