#pragma once

#include "Logger.h"
#include "core/PerformanceMonitor.h"
#include <chrono>
#include <functional>
#include <string>

class PerfTimer {
public:
    enum class Purpose { Log, Record };

    PerfTimer(std::string name, PerformanceMonitor &performanceMonitor,
              Purpose purpose = Purpose::Record)
        : _name(std::move(name)), _performanceMonitor(performanceMonitor), _purpose(purpose),
          _start(std::chrono::high_resolution_clock::now()) {}

    ~PerfTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - _start).count();

        if (_purpose == Purpose::Log) {
            LOG_DEBUG("Performance", "%s took %lld us", _name.c_str(), duration);
        } else {
            _performanceMonitor.record(_name, duration);
        }
    }

private:
    std::string _name;
    PerformanceMonitor &_performanceMonitor;
    Purpose _purpose;
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};