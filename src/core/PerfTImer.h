#pragma once

#include "Logger.h"
#include "core/ServiceLocator.h"
#include <chrono>
#include <functional>
#include <string>

class PerfTimer {
public:
    enum class Purpose { Log, Record };

    PerfTimer(std::string name, ServiceLocator &serviceLocator, Purpose purpose = Purpose::Record)
        : _name(std::move(name)), _serviceLocator(serviceLocator), _purpose(purpose),
          _start(std::chrono::high_resolution_clock::now()) {}

    ~PerfTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - _start).count();

        if (_purpose == Purpose::Log) {
            LOG_DEBUG("Performance", "%s took %lld us", _name.c_str(), duration);
        } else {
            _serviceLocator.performanceMonitor.record(_name, duration);
        }
    }

private:
    std::string _name;
    ServiceLocator &_serviceLocator;
    Purpose _purpose;
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};