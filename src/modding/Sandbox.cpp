#include "Sandbox.h"
#include <regex>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

namespace {
    constexpr size_t DEFAULT_MAX_MEMORY = 512 * 1024 * 1024; // 512MB
    constexpr size_t DEFAULT_MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB
    constexpr int DEFAULT_MAX_THREADS = 4;

    // List of potentially dangerous API calls
    const std::vector<std::string> DANGEROUS_PATTERNS = {
        "system\\s*\\(",
        "exec\\s*\\(",
        "popen\\s*\\(",
        "fork\\s*\\(",
        "\\<fstream\\>",
        "std::filesystem::remove",
        "std::remove",
        "DeleteFile",
        "CreateProcess"
    };
}

Sandbox::Sandbox()
    : m_maxMemoryUsage(DEFAULT_MAX_MEMORY)
    , m_maxFileSize(DEFAULT_MAX_FILE_SIZE)
    , m_maxThreads(DEFAULT_MAX_THREADS)
{
    // Initialize allowed API calls
    m_allowedAPICalls = {
        "GetResourceManager",
        "GetEventManager",
        "GetInputManager",
        "GetUIManager",
        "RegisterEventHandler",
        "LoadResource"
    };

    // Initialize allowed paths
    m_allowedPaths = {
        std::filesystem::path("plugins"),
        std::filesystem::path("assets"),
        std::filesystem::path("mods")
    };
}

Sandbox::~Sandbox() {
    ResetResourceLimits();
}

bool Sandbox::ValidateCode(const std::string& code) const {
    // Check for malicious patterns
    if (CheckForMaliciousCode(code)) {
        return false;
    }

    // Validate API usage
    if (!ValidateAPIUsage(code)) {
        return false;
    }

    return true;
}

bool Sandbox::IsPathSafe(const std::filesystem::path& path) const {
    // Normalize the path
    std::filesystem::path normalizedPath = std::filesystem::absolute(path).lexically_normal();
    
    // Check if the path is within allowed directories
    return std::any_of(m_allowedPaths.begin(), m_allowedPaths.end(),
        [&normalizedPath](const std::filesystem::path& allowedPath) {
            auto relativePath = std::filesystem::relative(normalizedPath, allowedPath);
            std::string relativeStr = relativePath.string();
            return relativeStr.substr(0, 2) != "..";
        });
}

bool Sandbox::CheckPermissions(const std::string& operation) const {
    return std::find(m_allowedAPICalls.begin(), m_allowedAPICalls.end(), operation) 
           != m_allowedAPICalls.end();
}

void Sandbox::LimitResources() {
#ifdef _WIN32
    // Windows-specific resource limitations
    HANDLE process = GetCurrentProcess();
    SIZE_T minWorkingSet = 0;
    SIZE_T maxWorkingSet = m_maxMemoryUsage;
    if (!SetProcessWorkingSetSize(process, minWorkingSet, maxWorkingSet)) {
        throw std::runtime_error("Failed to set process working set size");
    }

    // Set process priority to below normal to limit CPU usage
    SetPriorityClass(process, BELOW_NORMAL_PRIORITY_CLASS);
#else
    // Unix-like systems resource limitations using process nice value
    if (nice(10) == -1) { // Reduce process priority
        throw std::runtime_error("Failed to set process priority");
    }

    // Thread count limitation implemented through plugin validation
    // Memory limitations handled through application-level monitoring
#endif
}

void Sandbox::ResetResourceLimits() {
#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    SetProcessWorkingSetSize(process, 0, static_cast<SIZE_T>(-1));
    SetPriorityClass(process, NORMAL_PRIORITY_CLASS);
#else
    nice(0); // Reset process priority
#endif
}

bool Sandbox::CheckForMaliciousCode(const std::string& code) const {
    for (const auto& pattern : DANGEROUS_PATTERNS) {
        std::regex regex(pattern, std::regex::icase);
        if (std::regex_search(code, regex)) {
            return true;
        }
    }
    return false;
}

bool Sandbox::ValidateAPIUsage(const std::string& code) const {
    std::regex functionCallPattern(R"(\b\w+\s*\()");
    auto functionCalls = std::sregex_iterator(code.begin(), code.end(), functionCallPattern);
    auto end = std::sregex_iterator();

    for (auto it = functionCalls; it != end; ++it) {
        std::string functionName = it->str();
        functionName = functionName.substr(0, functionName.find('('));
        functionName.erase(std::remove_if(functionName.begin(), functionName.end(), ::isspace), 
                         functionName.end());

        if (!CheckPermissions(functionName)) {
            return false;
        }
    }
    return true;
} 