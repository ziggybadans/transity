#pragma once

#include <string>
#include <vector>
#include <filesystem>

class Sandbox {
public:
    Sandbox();
    ~Sandbox();

    /* Sandbox Security */
    bool ValidateCode(const std::string& code) const;
    bool IsPathSafe(const std::filesystem::path& path) const;
    bool CheckPermissions(const std::string& operation) const;

    /* Resource Management */
    void LimitResources();
    void ResetResourceLimits();

private:
    /* Security Settings */
    std::vector<std::string> m_allowedAPICalls;
    std::vector<std::filesystem::path> m_allowedPaths;
    
    /* Resource Limits */
    size_t m_maxMemoryUsage;
    size_t m_maxFileSize;
    int m_maxThreads;

    bool CheckForMaliciousCode(const std::string& code) const;
    bool ValidateAPIUsage(const std::string& code) const;
}; 