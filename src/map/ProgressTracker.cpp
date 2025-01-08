#include "ProgressTracker.h"
#include "../Debug.h"

ProgressTracker::ProgressTracker(std::function<void(float)> progressCallback)
    : m_callback(std::move(progressCallback))
    , m_currentProgress(0.0f)
    , m_currentStatus("Initializing...")
    , m_startTime(std::chrono::steady_clock::now())
    , m_statistics{}  // Zero-initialize all statistics members
{
    DEBUG_INFO("Progress tracker initialized");
    
    // Notify initial progress through callback if one exists
    if (m_callback) {
        try {
            m_callback(0.0f);
        } catch (const std::exception& e) {
            DEBUG_ERROR("Error in initial progress callback: ", e.what());
        }
    }
}

void ProgressTracker::UpdateProgress(float progress)
{
    // Validate progress is between 0.0 and 1.0
    if (progress < 0.0f || progress > 1.0f) {
        DEBUG_WARNING("Invalid progress value: ", progress, " (must be between 0.0 and 1.0)");
        return;
    }

    // Update current progress
    m_currentProgress = progress;

    // Calculate elapsed time
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = currentTime - m_startTime;
    m_statistics.elapsed = elapsed;

    // Update estimated time remaining if we have meaningful progress
    if (progress > 0.0f && progress < 1.0f) {
        float elapsedSeconds = std::chrono::duration<float>(elapsed).count();
        float progressRate = progress / elapsedSeconds;
        
        if (progressRate > 0.0f) {
            float remainingProgress = 1.0f - progress;
            m_statistics.estimatedTimeRemaining = remainingProgress / progressRate;
        }
    }

    // Invoke callback if provided
    if (m_callback) {
        try {
            m_callback(progress);
        } catch (const std::exception& e) {
            DEBUG_ERROR("Error in progress callback: ", e.what());
        }
    }

    // Log progress update
    DEBUG_DEBUG("Progress updated: ", 
                progress * 100.0f, "% complete, ",
                "estimated time remaining: ", 
                m_statistics.estimatedTimeRemaining, "s");
}

void ProgressTracker::UpdateBytes(size_t processedBytes, size_t totalBytes)
{
    // Update statistics with new byte counts
    m_statistics.processedBytes = processedBytes;
    m_statistics.totalBytes = totalBytes;

    // Calculate progress as ratio of processed to total bytes
    if (totalBytes > 0) {
        float progress = static_cast<float>(processedBytes) / static_cast<float>(totalBytes);
        
        // Update overall progress through existing method
        UpdateProgress(progress);
        
        // Invoke callback directly if progress update failed validation
        if (progress > 1.0f && m_callback) {
            try {
                m_callback(1.0f);
            } catch (const std::exception& e) {
                DEBUG_ERROR("Error in progress callback during byte update: ", e.what());
            }
        }
        
        // Log byte progress
        DEBUG_DEBUG("Bytes processed: ", 
                   processedBytes, "/", totalBytes,
                   " (", progress * 100.0f, "%)");
    } else {
        DEBUG_WARNING("UpdateBytes called with totalBytes = 0");
        
        // Notify callback of zero progress if totalBytes is invalid
        if (m_callback) {
            try {
                m_callback(0.0f);
            } catch (const std::exception& e) {
                DEBUG_ERROR("Error in progress callback during byte update: ", e.what());
            }
        }
    }
}

void ProgressTracker::SetStatus(const std::string& status)
{
    // Update current status message
    m_currentStatus = status;
    
    // Log status change in debug mode
    DEBUG_DEBUG("Progress status updated: ", status);
    
    // If we have a callback and progress, notify with current progress
    // This ensures UI updates when status changes even if progress hasn't
    if (m_callback && m_currentProgress > 0.0f) {
        m_callback(m_currentProgress);
    }
}

void ProgressTracker::Reset()
{
    // Reset progress tracking
    m_currentProgress = 0.0f;
    
    // Clear status message
    m_currentStatus.clear();
    
    // Reset start time to now
    m_startTime = std::chrono::steady_clock::now();
    
    // Reset all statistics
    m_statistics = Statistics{};  // Zero-initialize all members
    
    // Log the reset
    DEBUG_INFO("Progress tracker reset");
    
    // Notify callback of reset if one exists
    if (m_callback) {
        try {
            m_callback(0.0f);
        } catch (const std::exception& e) {
            DEBUG_ERROR("Error in progress callback during reset: ", e.what());
        }
    }
}

float ProgressTracker::GetProgress() const
{
    return m_currentProgress;
}

ProgressTracker::Statistics ProgressTracker::GetStatistics() const
{
    // Create a copy of current statistics
    Statistics stats = m_statistics;
    
    // Calculate current elapsed time
    auto currentTime = std::chrono::steady_clock::now();
    stats.elapsed = currentTime - m_startTime;
    
    // If we have progress, estimate remaining time
    if (m_currentProgress > 0.0f && m_currentProgress < 1.0f) {
        // Calculate rate of progress (progress per second)
        float elapsedSeconds = std::chrono::duration<float>(stats.elapsed).count();
        float progressRate = m_currentProgress / elapsedSeconds;
        
        // Estimate remaining time based on current rate
        if (progressRate > 0.0f) {
            float remainingProgress = 1.0f - m_currentProgress;
            stats.estimatedTimeRemaining = remainingProgress / progressRate;
        }
    }
    
    return stats;
}

std::string ProgressTracker::GetStatus() const
{
    return m_currentStatus;
} 