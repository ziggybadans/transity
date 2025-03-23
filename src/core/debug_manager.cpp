#include "transity/core/debug_manager.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <numeric>

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

void DebugManager::clearLogHistory() {
    logHistory.clear();
}

void DebugManager::setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

const std::deque<std::pair<LogLevel, std::string>>& DebugManager::getLogHistory() const {
    return logHistory;
}

void DebugManager::beginMetric(const std::string& name, const std::string& category, const std::string& unit) {
    activeMetrics[name] = std::chrono::steady_clock::now();
    metricCategories[name] = category;
    metricUnits[name] = unit;
}

void DebugManager::endMetric(const std::string& name) {
    auto it = activeMetrics.find(name);
    if (it != activeMetrics.end()) {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - it->second).count() / 1000.0;
        
        metrics.push_back({
            name,
            duration,
            end,
            metricCategories[name],
            metricUnits[name]
        });

        if (metrics.size() > MAX_METRICS_HISTORY) {
            metrics.erase(metrics.begin());
        }

        activeMetrics.erase(it);

        // Check performance thresholds
        auto thresholdIt = performanceThresholds.find(name);
        if (thresholdIt != performanceThresholds.end() && duration > thresholdIt->second) {
            log(LogLevel::Warning, 
                "Performance threshold exceeded for '" + name + 
                "': " + std::to_string(duration) + 
                " " + metricUnits[name] + 
                " (threshold: " + std::to_string(thresholdIt->second) + ")");
        }
    }
}

void DebugManager::clearMetrics() {
    metrics.clear();
    activeMetrics.clear();
}

double DebugManager::getAverageMetric(const std::string& name, size_t sampleCount) const {
    std::vector<double> values;
    for (auto it = metrics.rbegin(); it != metrics.rend() && values.size() < sampleCount; ++it) {
        if (it->name == name) {
            values.push_back(it->value);
        }
    }
    
    if (values.empty()) return 0.0;
    
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

std::vector<PerformanceMetric> DebugManager::getMetricsByCategory(const std::string& category) const {
    std::vector<PerformanceMetric> result;
    std::copy_if(metrics.begin(), metrics.end(), std::back_inserter(result),
                 [&](const PerformanceMetric& metric) { return metric.category == category; });
    return result;
}

void DebugManager::updateSystemMetrics(const SystemMetrics& metrics) {
    latestSystemMetrics = metrics;
    systemMetricsHistory.push_back(metrics);
    
    if (systemMetricsHistory.size() > MAX_SYSTEM_METRICS_HISTORY) {
        systemMetricsHistory.erase(systemMetricsHistory.begin());
    }
}

const SystemMetrics& DebugManager::getLatestSystemMetrics() const {
    return latestSystemMetrics;
}

std::vector<SystemMetrics> DebugManager::getSystemMetricsHistory(size_t count) const {
    if (count >= systemMetricsHistory.size()) {
        return systemMetricsHistory;
    }
    return std::vector<SystemMetrics>(
        systemMetricsHistory.end() - count,
        systemMetricsHistory.end()
    );
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

void DebugManager::removeDebugInfo(const std::string& key) {
    debugInfo.erase(key);
}

const std::map<std::string, std::string>& DebugManager::getDebugInfo() const {
    return debugInfo;
}

void DebugManager::clearDebugInfo() {
    debugInfo.clear();
}

void DebugManager::createDebugSection(const std::string& name) {
    if (debugSections.find(name) == debugSections.end()) {
        debugSections[name] = std::map<std::string, std::string>();
    }
}

void DebugManager::addDebugInfoToSection(const std::string& section, const std::string& key, const std::string& value) {
    if (debugSections.find(section) == debugSections.end()) {
        createDebugSection(section);
    }
    debugSections[section][key] = value;
}

const std::map<std::string, std::string>& DebugManager::getDebugSection(const std::string& name) const {
    static const std::map<std::string, std::string> empty;
    auto it = debugSections.find(name);
    return it != debugSections.end() ? it->second : empty;
}

void DebugManager::registerCommand(const std::string& name, DebugCommand command, const std::string& description) {
    debugCommands[name] = command;
    commandDescriptions[name] = description;
}

bool DebugManager::executeCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmdName;
    iss >> cmdName;

    auto it = debugCommands.find(cmdName);
    if (it == debugCommands.end()) {
        log(LogLevel::Warning, "Unknown command: " + cmdName);
        return false;
    }

    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }

    try {
        it->second(args);
        log(LogLevel::Debug, "Successfully executed command: " + cmdName);
        return true;
    } catch (const std::exception& e) {
        log(LogLevel::Error, "Error executing command '" + cmdName + "': " + e.what());
        return false;
    }
}

std::map<std::string, std::string> DebugManager::getCommandList() const {
    return commandDescriptions;
}

void DebugManager::setSystemState(const std::string& system, const std::string& state) {
    systemStates[system] = state;
    log(LogLevel::Debug, "System '" + system + "' state changed to: " + state);
}

std::string DebugManager::getSystemState(const std::string& system) const {
    auto it = systemStates.find(system);
    return it != systemStates.end() ? it->second : "";
}

void DebugManager::clearSystemState(const std::string& system) {
    systemStates.erase(system);
}

const std::map<std::string, std::string>& DebugManager::getAllSystemStates() const {
    return systemStates;
}

void DebugManager::setPerformanceThreshold(const std::string& metric, double threshold) {
    performanceThresholds[metric] = threshold;
}

bool DebugManager::isPerformanceThresholdExceeded(const std::string& metric) const {
    auto metricIt = std::find_if(metrics.rbegin(), metrics.rend(),
                                [&](const PerformanceMetric& m) { return m.name == metric; });
                                
    if (metricIt == metrics.rend()) return false;
    
    auto thresholdIt = performanceThresholds.find(metric);
    if (thresholdIt == performanceThresholds.end()) return false;
    
    return metricIt->value > thresholdIt->second;
}

const std::vector<PerformanceMetric>& DebugManager::getMetrics() const {
    return metrics;
}

} // namespace core
} // namespace transity 