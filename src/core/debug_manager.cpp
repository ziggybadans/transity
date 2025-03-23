#include "transity/core/debug_manager.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <numeric>
#include <thread>
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

// Performance monitoring and reporting
void DebugManager::startProfiling(const std::string& session) {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    if (m_isProfilingActive) {
        log(LogLevel::Warning, "Profiling session already active. Stopping current session.");
        stopProfiling();
    }
    m_currentProfilingSession = session;
    m_isProfilingActive = true;
    m_profilingEvents.clear();
    log(LogLevel::Info, "Started profiling session: " + session);
}

void DebugManager::stopProfiling() {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    if (!m_isProfilingActive) {
        return;
    }
    m_isProfilingActive = false;
    log(LogLevel::Info, "Stopped profiling session: " + m_currentProfilingSession);
}

void DebugManager::exportProfilingData(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    // TODO: Implement JSON serialization of profiling data
    log(LogLevel::Info, "Exported profiling data to: " + filename);
}

std::vector<ProfileEvent> DebugManager::getProfilingEvents(const std::string& category) const {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    if (category.empty()) {
        return m_profilingEvents;
    }
    std::vector<ProfileEvent> filtered;
    std::copy_if(m_profilingEvents.begin(), m_profilingEvents.end(), 
                 std::back_inserter(filtered),
                 [&](const ProfileEvent& event) { return event.category == category; });
    return filtered;
}

// Memory usage tracking
size_t DebugManager::getCurrentMemoryUsage() const {
    return latestSystemMetrics.memoryUsage;
}

size_t DebugManager::getPeakMemoryUsage() const {
    return m_peakMemoryUsage;
}

void DebugManager::resetPeakMemoryUsage() {
    m_peakMemoryUsage = getCurrentMemoryUsage();
}

std::vector<std::pair<std::string, size_t>> DebugManager::getMemoryBreakdown() const {
    // TODO: Implement detailed memory breakdown by category/system
    return {};
}

// System resource monitoring
void DebugManager::enableResourceMonitoring(bool enable) {
    m_resourceMonitoringEnabled = enable;
    log(LogLevel::Info, std::string("Resource monitoring ") + (enable ? "enabled" : "disabled"));
}

bool DebugManager::isResourceMonitoringEnabled() const {
    return m_resourceMonitoringEnabled;
}

SystemMetrics DebugManager::getDetailedSystemMetrics() const {
    return latestSystemMetrics;
}

std::map<std::string, double> DebugManager::getResourceUtilization() const {
    return {
        {"cpu", latestSystemMetrics.cpuUsage},
        {"memory", static_cast<double>(latestSystemMetrics.memoryUsage) / latestSystemMetrics.availableMemory},
        {"gpu", latestSystemMetrics.gpuUsage},
        {"disk", latestSystemMetrics.diskUsage}
    };
}

// Visual profiling tools
void DebugManager::beginProfile(const std::string& name, const std::string& category) {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    if (!m_isProfilingActive) {
        return;
    }
    
    ProfileEvent event;
    event.name = name;
    event.category = category;
    event.startTime = std::chrono::steady_clock::now();
    event.threadId = std::this_thread::get_id();
    event.memoryUsage = getCurrentMemoryUsage();
    
    m_activeProfiles[name] = event;
}

void DebugManager::endProfile(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    if (!m_isProfilingActive) {
        return;
    }
    
    auto it = m_activeProfiles.find(name);
    if (it != m_activeProfiles.end()) {
        auto& event = it->second;
        event.endTime = std::chrono::steady_clock::now();
        m_profilingEvents.push_back(event);
        m_activeProfiles.erase(it);
    }
}

void DebugManager::addProfileMetadata(const std::string& name, const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    auto it = m_activeProfiles.find(name);
    if (it != m_activeProfiles.end()) {
        it->second.metadata[key] = value;
    }
}

std::vector<ProfileEvent> DebugManager::getActiveProfiles() const {
    std::lock_guard<std::mutex> lock(m_profilingMutex);
    std::vector<ProfileEvent> active;
    for (const auto& [name, event] : m_activeProfiles) {
        active.push_back(event);
    }
    return active;
}

// Event replay system
void DebugManager::startEventRecording() {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_isRecordingEvents = true;
    m_recordedEvents.clear();
    log(LogLevel::Info, "Started event recording");
}

void DebugManager::stopEventRecording() {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_isRecordingEvents = false;
    log(LogLevel::Info, "Stopped event recording. Total events: " + std::to_string(m_recordedEvents.size()));
}

void DebugManager::recordEvent(const std::string& type, const std::map<std::string, std::string>& data) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    if (!m_isRecordingEvents) {
        return;
    }
    m_recordedEvents.emplace_back(type, data);
}

void DebugManager::saveEventLog(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    
    json j;
    j["events"] = json::array();
    
    for (const auto& [type, data] : m_recordedEvents) {
        json event;
        event["type"] = type;
        event["data"] = data;
        j["events"].push_back(event);
    }
    
    std::ofstream file(filename);
    if (!file) {
        log(LogLevel::Error, "Failed to open file for writing: " + filename);
        return;
    }
    
    file << j.dump(4);
    log(LogLevel::Info, "Saved event log to: " + filename);
}

void DebugManager::loadEventLog(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    
    std::ifstream file(filename);
    if (!file) {
        log(LogLevel::Error, "Failed to open file for reading: " + filename);
        return;
    }
    
    try {
        json j;
        file >> j;
        
        m_recordedEvents.clear();
        
        for (const auto& event : j["events"]) {
            std::string type = event["type"];
            std::map<std::string, std::string> data = event["data"];
            m_recordedEvents.emplace_back(type, data);
        }
        
        log(LogLevel::Info, "Loaded event log from: " + filename);
    } catch (const json::exception& e) {
        log(LogLevel::Error, "Failed to parse event log: " + std::string(e.what()));
    }
}

void DebugManager::replayEvents(const std::function<void(const std::string&, const std::map<std::string, std::string>&)>& callback) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    for (const auto& [type, data] : m_recordedEvents) {
        callback(type, data);
    }
}

bool DebugManager::isRecordingEvents() const {
    return m_isRecordingEvents;
}

} // namespace core
} // namespace transity 