#pragma once

#include <functional>
#include <string>
#include <chrono>

/**
 * @brief Handles progress tracking and timing during map loading
 */
class ProgressTracker {
public:
    struct Statistics {
        size_t totalBytes{0};
        size_t processedBytes{0};
        std::chrono::steady_clock::duration elapsed{};
        float estimatedTimeRemaining{0.0f};
    };

    explicit ProgressTracker(std::function<void(float)> progressCallback);
    ~ProgressTracker() = default;
    
    void UpdateProgress(float progress);
    void UpdateBytes(size_t processedBytes, size_t totalBytes);
    void SetStatus(const std::string& status);
    void Reset();

    float GetProgress() const;
    Statistics GetStatistics() const;
    std::string GetStatus() const;

private:
    std::function<void(float)> m_callback;
    float m_currentProgress{0.0f};
    std::string m_currentStatus;
    std::chrono::steady_clock::time_point m_startTime;
    Statistics m_statistics;
}; 