#pragma once

#include "../Logger.h"
#include <chrono>

class PerfTimer {
public:
    PerfTimer(const char* name)
        : m_name(name), m_start(std::chrono::high_resolution_clock::now()) {}

    ~PerfTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
        LOG_DEBUG("Performance", "%s took %lld us", m_name, duration);
    }

private:
    const char* m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};