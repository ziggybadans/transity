#include "Profiler.h"
#include "../Debug.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

// Initialize static members
std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> Profiler::m_startTimes;
std::unordered_map<std::string, double> Profiler::m_profiles;
std::mutex Profiler::m_mutex;
bool Profiler::m_enabled = true;

void Profiler::BeginProfile(const std::string& name) {
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_startTimes[name] = std::chrono::high_resolution_clock::now();
}

void Profiler::EndProfile(const std::string& name) {
    if (!m_enabled) return;

    auto endTime = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto startIter = m_startTimes.find(name);
    if (startIter != m_startTimes.end()) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startIter->second).count() / 1000.0; // Convert to milliseconds
        
        if (m_profiles.find(name) == m_profiles.end()) {
            m_profiles[name] = duration;
        } else {
            // Running average
            m_profiles[name] = (m_profiles[name] * 0.95) + (duration * 0.05);
        }
        
        m_startTimes.erase(startIter);
    }
}

void Profiler::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_startTimes.clear();
    m_profiles.clear();
}

void Profiler::Enable() {
    m_enabled = true;
}

void Profiler::Disable() {
    m_enabled = false;
}

const std::unordered_map<std::string, double>& Profiler::GetProfiles() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_profiles;
}

std::vector<Profiler::ProfileData> Profiler::GetSortedProfiles() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<ProfileData> sortedData;
    sortedData.reserve(m_profiles.size());
    
    for (const auto& [name, duration] : m_profiles) {
        sortedData.push_back({name, duration});
    }
    
    std::sort(sortedData.begin(), sortedData.end(),
        [](const ProfileData& a, const ProfileData& b) {
            return a.duration > b.duration;
        });
    
    return sortedData;
} 