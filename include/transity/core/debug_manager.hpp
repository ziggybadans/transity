#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
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

    static constexpr size_t MAX_LOG_HISTORY = 1000;
    static constexpr size_t MAX_METRICS_HISTORY = 1000;
    static constexpr size_t MAX_SYSTEM_METRICS_HISTORY = 1000;
};

} // namespace core
} // namespace transity 