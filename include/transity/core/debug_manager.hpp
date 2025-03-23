#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include "transity/core/error.hpp"

namespace transity {
namespace core {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

struct PerformanceMetric {
    std::string name;
    double value;
    std::chrono::steady_clock::time_point timestamp;
    std::string category;  // Added category for better organization
    std::string unit;      // Added unit for better context
};

struct SystemMetrics {
    double cpuUsage;
    size_t memoryUsage;
    double frameTime;
    std::chrono::steady_clock::time_point timestamp;
    // New fields for enhanced system monitoring
    size_t availableMemory;
    size_t peakMemoryUsage;
    double gpuUsage;
    double diskUsage;
    size_t diskReadBytes;
    size_t diskWriteBytes;
    size_t networkReadBytes;
    size_t networkWriteBytes;
};

struct ProfileEvent {
    std::string name;
    std::string category;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    std::thread::id threadId;
    size_t memoryUsage;
    std::map<std::string, std::string> metadata;
};

class DebugManager {
public:
    static DebugManager& getInstance();

    // Logging
    void log(LogLevel level, const std::string& message);
    void setLogLevel(LogLevel level);
    const std::deque<std::pair<LogLevel, std::string>>& getLogHistory() const;
    void clearLogHistory();

    // Performance metrics
    void beginMetric(const std::string& name, const std::string& category = "default", const std::string& unit = "ms");
    void endMetric(const std::string& name);
    const std::vector<PerformanceMetric>& getMetrics() const;
    void clearMetrics();
    double getAverageMetric(const std::string& name, size_t sampleCount = 100) const;
    std::vector<PerformanceMetric> getMetricsByCategory(const std::string& category) const;

    // System metrics
    void updateSystemMetrics(const SystemMetrics& metrics);
    const SystemMetrics& getLatestSystemMetrics() const;
    std::vector<SystemMetrics> getSystemMetricsHistory(size_t count = 100) const;

    // Debug overlay
    void setDebugOverlayEnabled(bool enabled);
    bool isDebugOverlayEnabled() const;
    void addDebugInfo(const std::string& key, const std::string& value);
    void removeDebugInfo(const std::string& key);
    const std::map<std::string, std::string>& getDebugInfo() const;
    void clearDebugInfo();

    // Debug sections (for organizing debug info)
    void createDebugSection(const std::string& name);
    void addDebugInfoToSection(const std::string& section, const std::string& key, const std::string& value);
    const std::map<std::string, std::string>& getDebugSection(const std::string& name) const;

    // New debug overlay methods
    void addMetric(const std::string& name, std::function<std::string()> valueProvider);
    void removeMetric(const std::string& name);
    std::string getDebugOverlayContent() const;
    void beginSection(const std::string& name);
    void endSection();

    // Debug commands
    using DebugCommand = std::function<void(const std::vector<std::string>&)>;
    void registerCommand(const std::string& name, DebugCommand command, const std::string& description);
    bool executeCommand(const std::string& command);
    std::map<std::string, std::string> getCommandList() const;  // Returns command name -> description

    // System state
    void setSystemState(const std::string& system, const std::string& state);
    std::string getSystemState(const std::string& system) const;
    void clearSystemState(const std::string& system);
    const std::map<std::string, std::string>& getAllSystemStates() const;

    // Performance thresholds
    void setPerformanceThreshold(const std::string& metric, double threshold);
    bool isPerformanceThresholdExceeded(const std::string& metric) const;

    // Performance monitoring and reporting
    void startProfiling(const std::string& session);
    void stopProfiling();
    void exportProfilingData(const std::string& filename);
    std::vector<ProfileEvent> getProfilingEvents(const std::string& category = "") const;
    
    // Memory usage tracking
    size_t getCurrentMemoryUsage() const;
    size_t getPeakMemoryUsage() const;
    void resetPeakMemoryUsage();
    std::vector<std::pair<std::string, size_t>> getMemoryBreakdown() const;
    
    // System resource monitoring
    void enableResourceMonitoring(bool enable);
    bool isResourceMonitoringEnabled() const;
    SystemMetrics getDetailedSystemMetrics() const;
    std::map<std::string, double> getResourceUtilization() const;
    
    // Visual profiling tools
    void beginProfile(const std::string& name, const std::string& category);
    void endProfile(const std::string& name);
    void addProfileMetadata(const std::string& name, const std::string& key, const std::string& value);
    std::vector<ProfileEvent> getActiveProfiles() const;
    
    // Event replay system
    void startEventRecording();
    void stopEventRecording();
    void recordEvent(const std::string& type, const std::map<std::string, std::string>& data);
    void saveEventLog(const std::string& filename);
    void loadEventLog(const std::string& filename);
    void replayEvents(const std::function<void(const std::string&, const std::map<std::string, std::string>&)>& callback);
    bool isRecordingEvents() const;

private:
    DebugManager();
    ~DebugManager() = default;
    DebugManager(const DebugManager&) = delete;
    DebugManager& operator=(const DebugManager&) = delete;

    LogLevel currentLogLevel = LogLevel::Info;
    std::deque<std::pair<LogLevel, std::string>> logHistory;
    std::vector<PerformanceMetric> metrics;
    std::vector<SystemMetrics> systemMetricsHistory;
    bool debugOverlayEnabled = false;
    std::map<std::string, std::string> debugInfo;
    std::map<std::string, std::map<std::string, std::string>> debugSections;
    std::map<std::string, DebugCommand> debugCommands;
    std::map<std::string, std::string> commandDescriptions;
    std::map<std::string, std::string> systemStates;
    std::map<std::string, std::chrono::steady_clock::time_point> activeMetrics;
    std::map<std::string, std::string> metricCategories;
    std::map<std::string, std::string> metricUnits;
    std::map<std::string, double> performanceThresholds;
    SystemMetrics latestSystemMetrics{};

    // New private members
    bool m_resourceMonitoringEnabled{false};
    bool m_isRecordingEvents{false};
    bool m_isProfilingActive{false};
    std::string m_currentProfilingSession;
    std::vector<ProfileEvent> m_profilingEvents;
    std::map<std::string, ProfileEvent> m_activeProfiles;
    std::vector<std::pair<std::string, std::map<std::string, std::string>>> m_recordedEvents;
    size_t m_peakMemoryUsage{0};
    mutable std::mutex m_profilingMutex;
    mutable std::mutex m_eventMutex;

    // New private members for debug overlay
    std::map<std::string, std::function<std::string()>> m_metricProviders;
    std::vector<std::string> m_activeSections;
    mutable std::mutex m_overlayMutex;

    static constexpr size_t MAX_LOG_HISTORY = 1000;
    static constexpr size_t MAX_METRICS_HISTORY = 1000;
    static constexpr size_t MAX_SYSTEM_METRICS_HISTORY = 1000;
};

} // namespace core
} // namespace transity 