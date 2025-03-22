#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>

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
};

class DebugManager {
public:
    static DebugManager& getInstance();

    // Logging
    void log(LogLevel level, const std::string& message);
    void setLogLevel(LogLevel level);
    const std::deque<std::pair<LogLevel, std::string>>& getLogHistory() const;

    // Performance metrics
    void beginMetric(const std::string& name);
    void endMetric(const std::string& name);
    const std::vector<PerformanceMetric>& getMetrics() const;

    // Debug overlay
    void setDebugOverlayEnabled(bool enabled);
    bool isDebugOverlayEnabled() const;
    void addDebugInfo(const std::string& key, const std::string& value);
    const std::map<std::string, std::string>& getDebugInfo() const;

    // Debug commands
    using DebugCommand = std::function<void(const std::vector<std::string>&)>;
    void registerCommand(const std::string& name, DebugCommand command);
    bool executeCommand(const std::string& command);

    // System state
    void setSystemState(const std::string& system, const std::string& state);
    std::string getSystemState(const std::string& system) const;

private:
    DebugManager();
    ~DebugManager() = default;
    DebugManager(const DebugManager&) = delete;
    DebugManager& operator=(const DebugManager&) = delete;

    LogLevel currentLogLevel = LogLevel::Info;
    std::deque<std::pair<LogLevel, std::string>> logHistory;
    std::vector<PerformanceMetric> metrics;
    bool debugOverlayEnabled = false;
    std::map<std::string, std::string> debugInfo;
    std::map<std::string, DebugCommand> debugCommands;
    std::map<std::string, std::string> systemStates;
    std::map<std::string, std::chrono::steady_clock::time_point> activeMetrics;

    static constexpr size_t MAX_LOG_HISTORY = 1000;
};

} // namespace core
} // namespace transity 