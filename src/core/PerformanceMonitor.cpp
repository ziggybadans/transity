#include "PerformanceMonitor.h"

void PerformanceMonitor::record(const std::string &name, long long duration) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto &history = _data[name];
    history.push_back(static_cast<float>(duration));
    if (history.size() > _historySize) {
        // Erase the oldest element to maintain a fixed-size window
        history.erase(history.begin());
    }
}

const std::vector<float> &PerformanceMonitor::getHistory(const std::string &name) {
    std::lock_guard<std::mutex> lock(_mutex);
    // Return an empty vector if the key doesn't exist, to avoid creating new entries on read
    if (_data.find(name) == _data.end()) {
        static const std::vector<float> empty;
        return empty;
    }
    return _data[name];
}
