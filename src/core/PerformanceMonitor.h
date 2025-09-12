#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

class PerformanceMonitor {
public:
    void record(const std::string& name, long long duration);
    const std::vector<float>& getHistory(const std::string& name);

private:
    std::mutex _mutex;
    std::unordered_map<std::string, std::vector<float>> _data;
    static const size_t _historySize = 120; // Store last 120 frames
};