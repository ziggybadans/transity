#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>

class Profiler {
public:
    struct ProfileData {
        std::string name;
        double duration;
    };

    static void BeginProfile(const std::string& name);
    static void EndProfile(const std::string& name);
    static void Reset();
    static void Enable();
    static void Disable();
    
    static std::vector<ProfileData> GetSortedProfiles();
    static const std::unordered_map<std::string, double>& GetProfiles();

private:
    static std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_startTimes;
    static std::unordered_map<std::string, double> m_profiles;
    static std::mutex m_mutex;
    static bool m_enabled;
};

// RAII-style profiler helper
class ScopedProfiler {
public:
    explicit ScopedProfiler(const std::string& name) : m_name(name) {
        Profiler::BeginProfile(m_name);
    }
    
    ~ScopedProfiler() {
        Profiler::EndProfile(m_name);
    }

private:
    std::string m_name;
};

// Convenience macro for scope-based profiling
#define PROFILE_SCOPE(name) ScopedProfiler scopedProfiler##__LINE__(name) 