#include "transity/core/debug_manager.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace transity {
namespace core {

DebugManager& DebugManager::getInstance() {
    static DebugManager instance;
    return instance;
}

DebugManager::DebugManager() {}

void DebugManager::log(LogLevel level, const std::string& message) {
    if (level >= currentLogLevel) {
        logHistory.emplace_back(level, message);
        if (logHistory.size() > MAX_LOG_HISTORY) {
            logHistory.pop_front();
        }

        // Output to console for immediate feedback
        const char* levelStr;
        switch (level) {
            case LogLevel::Debug:   levelStr = "DEBUG"; break;
            case LogLevel::Info:    levelStr = "INFO"; break;
            case LogLevel::Warning: levelStr = "WARNING"; break;
            case LogLevel::Error:   levelStr = "ERROR"; break;
        }
        std::cout << "[" << levelStr << "] " << message << std::endl;
    }
}

void DebugManager::setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

const std::deque<std::pair<LogLevel, std::string>>& DebugManager::getLogHistory() const {
    return logHistory;
}

void DebugManager::beginMetric(const std::string& name) {
    activeMetrics[name] = std::chrono::steady_clock::now();
}

void DebugManager::endMetric(const std::string& name) {
    auto it = activeMetrics.find(name);
    if (it != activeMetrics.end()) {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - it->second).count() / 1000.0;
        metrics.push_back({name, duration, end});
        activeMetrics.erase(it);
    }
}

const std::vector<PerformanceMetric>& DebugManager::getMetrics() const {
    return metrics;
}

void DebugManager::setDebugOverlayEnabled(bool enabled) {
    debugOverlayEnabled = enabled;
}

bool DebugManager::isDebugOverlayEnabled() const {
    return debugOverlayEnabled;
}

void DebugManager::addDebugInfo(const std::string& key, const std::string& value) {
    debugInfo[key] = value;
}

const std::map<std::string, std::string>& DebugManager::getDebugInfo() const {
    return debugInfo;
}

void DebugManager::registerCommand(const std::string& name, DebugCommand command) {
    debugCommands[name] = command;
}

bool DebugManager::executeCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmdName;
    iss >> cmdName;

    auto it = debugCommands.find(cmdName);
    if (it == debugCommands.end()) {
        return false;
    }

    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    try {
        it->second(args);
        return true;
    } catch (const std::exception& e) {
        log(LogLevel::Error, "Error executing command '" + cmdName + "': " + e.what());
        return false;
    }
}

void DebugManager::setSystemState(const std::string& system, const std::string& state) {
    systemStates[system] = state;
}

std::string DebugManager::getSystemState(const std::string& system) const {
    auto it = systemStates.find(system);
    return it != systemStates.end() ? it->second : "";
}

} // namespace core
} // namespace transity 