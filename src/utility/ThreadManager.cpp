#include "ThreadManager.h"
#include <algorithm>
#include <future>

ThreadManager::ThreadManager(size_t numThreads)
    : m_isMonitoring(true)
{
    numThreads = std::min(numThreads, MAX_THREADS);
    InitializeThreads(numThreads);
    
    // Start monitoring thread
    m_monitorThread = std::thread(&ThreadManager::MonitorThreadHealth, this);
}

ThreadManager::~ThreadManager() {
    Shutdown();
}

void ThreadManager::InitializeThreads(size_t numThreads) {
    m_threadPool = std::make_unique<ThreadPool>(numThreads);
    m_threadStats.resize(numThreads);
    
    for (size_t i = 0; i < numThreads; ++i) {
        m_threadStats[i].name = "Thread-" + std::to_string(i);
        m_threadStats[i].priority = ThreadPriority::Normal;
        m_threadStats[i].startTime = std::chrono::steady_clock::now();
    }
}

void ThreadManager::MonitorThreadHealth() {
    while (m_isMonitoring) {
        std::this_thread::sleep_for(MONITOR_INTERVAL);
        
        for (size_t i = 0; i < m_threadStats.size(); ++i) {
            auto& stats = m_threadStats[i];
            
            // Check for thread health issues
            if (stats.tasksQueued > 0 && 
                static_cast<double>(stats.tasksFailed) / stats.tasksQueued > HEALTH_CHECK_THRESHOLD) {
                DEBUG_WARNING("Thread ", i, " has high failure rate. Consider investigation.");
            }
            
            // Check for thread starvation
            if (stats.tasksProcessed == 0 && stats.tasksQueued > 0) {
                DEBUG_WARNING("Thread ", i, " might be stalled.");
            }
        }
    }
}

void ThreadManager::UpdateThreadStats(size_t threadIndex, const std::string& taskName,
                                   std::chrono::steady_clock::time_point startTime) {
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    
    std::lock_guard<std::mutex> lock(m_statsMutex);
    auto& stats = m_threadStats[threadIndex];
    
    // Update average processing time using exponential moving average
    double currentAvg = stats.averageProcessingTime;
    stats.averageProcessingTime = (currentAvg * 0.95) + (duration * 0.05);
    stats.tasksQueued--;
}

void ThreadManager::Shutdown() {
    m_isMonitoring = false;
    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }
    
    if (m_threadPool) {
        m_threadPool->shutdown();
    }
}

void ThreadManager::EmergencyStop() {
    DEBUG_WARNING("Emergency stop initiated");
    
    // Stop monitoring
    m_isMonitoring = false;
    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }

    // Shutdown current thread pool
    if (m_threadPool) {
        m_threadPool->shutdown();
    }

    // Clear all stats
    std::lock_guard<std::mutex> lock(m_statsMutex);
    for (auto& stats : m_threadStats) {
        stats.tasksQueued = 0;
        stats.tasksProcessed = 0;
        stats.tasksFailed = 0;
        stats.averageProcessingTime = 0.0;
    }

    // Create new thread pool
    InitializeThreads(m_threadStats.size());
    DEBUG_INFO("Emergency stop completed, thread pool reinitialized");
}

void ThreadManager::SetThreadPriority(size_t threadIndex, ThreadPriority priority) {
    if (threadIndex >= m_threadStats.size()) {
        DEBUG_ERROR("Invalid thread index: ", threadIndex);
        return;
    }

    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_threadStats[threadIndex].priority = priority;
    DEBUG_INFO("Thread ", threadIndex, " priority set to ", static_cast<int>(priority));
}

void ThreadManager::AdjustThreadCount(size_t newCount) {
    if (newCount == 0 || newCount > MAX_THREADS) {
        DEBUG_ERROR("Invalid thread count: ", newCount);
        return;
    }

    std::lock_guard<std::mutex> lock(m_statsMutex);
    
    // Create new thread pool with desired count
    auto newThreadPool = std::make_unique<ThreadPool>(newCount);
    m_threadPool = std::move(newThreadPool);
    
    // Adjust stats vector size
    m_threadStats.resize(newCount);
    
    // Initialize new thread stats
    for (size_t i = 0; i < newCount; ++i) {
        if (m_threadStats[i].name.empty()) {
            m_threadStats[i].name = "Thread-" + std::to_string(i);
            m_threadStats[i].priority = ThreadPriority::Normal;
            m_threadStats[i].startTime = std::chrono::steady_clock::now();
        }
    }
    
    DEBUG_INFO("Thread count adjusted to ", newCount);
}

void ThreadManager::PauseThread(size_t threadIndex) {
    if (threadIndex >= m_threadStats.size()) {
        DEBUG_ERROR("Invalid thread index: ", threadIndex);
        return;
    }

    std::lock_guard<std::mutex> lock(m_statsMutex);
    // Mark thread as paused in stats
    m_threadStats[threadIndex].priority = ThreadPriority::Low;
    DEBUG_INFO("Thread ", threadIndex, " paused");
}

void ThreadManager::ResumeThread(size_t threadIndex) {
    if (threadIndex >= m_threadStats.size()) {
        DEBUG_ERROR("Invalid thread index: ", threadIndex);
        return;
    }

    std::lock_guard<std::mutex> lock(m_statsMutex);
    // Restore thread priority to normal
    m_threadStats[threadIndex].priority = ThreadPriority::Normal;
    DEBUG_INFO("Thread ", threadIndex, " resumed");
}

const ThreadStats& ThreadManager::GetThreadStats(size_t threadIndex) const {
    if (threadIndex >= m_threadStats.size()) {
        throw std::out_of_range("Invalid thread index");
    }
    return m_threadStats[threadIndex];
}

std::vector<ThreadStats> ThreadManager::GetAllThreadStats() const {
    std::vector<ThreadStats> stats;
    stats.reserve(m_threadStats.size());
    
    for (const auto& stat : m_threadStats) {
        ThreadStats copy;
        copy.name = stat.name;
        copy.priority = stat.priority;
        copy.startTime = stat.startTime;
        copy.tasksProcessed.store(stat.tasksProcessed.load());
        copy.tasksQueued.store(stat.tasksQueued.load());
        copy.tasksFailed.store(stat.tasksFailed.load());
        copy.averageProcessingTime.store(stat.averageProcessingTime.load());
        stats.push_back(std::move(copy));
    }
    
    return stats;
}

float ThreadManager::GetThreadUtilization() const {
    size_t totalTasks = 0;
    size_t completedTasks = 0;

    for (const auto& stats : m_threadStats) {
        totalTasks += stats.tasksQueued + stats.tasksProcessed;
        completedTasks += stats.tasksProcessed;
    }

    return totalTasks > 0 ? static_cast<float>(completedTasks) / totalTasks : 0.0f;
}

size_t ThreadManager::GetActiveThreadCount() const {
    return std::count_if(m_threadStats.begin(), m_threadStats.end(),
        [](const ThreadStats& stats) {
            return stats.tasksQueued > 0 || stats.tasksProcessed > 0;
        });
} 