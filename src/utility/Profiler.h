#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

class Profiler {
public:
    static void BeginProfile(const std::string& name);
    static void EndProfile(const std::string& name);
    static void Reset();
    static const std::unordered_map<std::string, double>& GetProfiles();

private:
    static std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_startTimes;
    static std::unordered_map<std::string, double> m_profiles;
}; 