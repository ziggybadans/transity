#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <chrono>
#include "ThreadPool.h"
#include "../Debug.h"

enum class ThreadPriority
{
    Low,
    Normal,
    High,
    Critical
};

struct ThreadStats
{
    std::string name;
    ThreadPriority priority;
    std::chrono::steady_clock::time_point startTime;
    std::atomic<size_t> tasksProcessed;
    std::atomic<size_t> tasksQueued;
    std::atomic<size_t> tasksFailed;
    std::atomic<double> averageProcessingTime;

    // Custom copy constructor
    ThreadStats()
        : tasksProcessed(0), tasksQueued(0), tasksFailed(0), averageProcessingTime(0.0)
    {
    }

    // Copy constructor
    ThreadStats(const ThreadStats &other)
        : name(other.name), priority(other.priority), startTime(other.startTime), tasksProcessed(other.tasksProcessed.load()), tasksQueued(other.tasksQueued.load()), tasksFailed(other.tasksFailed.load()), averageProcessingTime(other.averageProcessingTime.load())
    {
    }

    // Assignment operator
    ThreadStats &operator=(const ThreadStats &other)
    {
        if (this != &other)
        {
            name = other.name;
            priority = other.priority;
            startTime = other.startTime;
            tasksProcessed.store(other.tasksProcessed.load());
            tasksQueued.store(other.tasksQueued.load());
            tasksFailed.store(other.tasksFailed.load());
            averageProcessingTime.store(other.averageProcessingTime.load());
        }
        return *this;
    }
};

class ThreadManager
{
public:
    explicit ThreadManager(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadManager();

    // Task submission methods
    template <typename F, typename... Args>
    void EnqueueTask(const std::string &taskName, ThreadPriority priority, F &&f, Args &&...args)
    {
        auto taskStartTime = std::chrono::steady_clock::now();
        size_t threadIndex = m_threadStats.size() - 1; // Default to last thread

        // Find the most appropriate thread based on priority and load
        for (size_t i = 0; i < m_threadStats.size(); ++i)
        {
            if (m_threadStats[i].priority == priority &&
                m_threadStats[i].tasksQueued < m_threadStats[threadIndex].tasksQueued)
            {
                threadIndex = i;
            }
        }

        // Create a bound function that contains all arguments
        auto boundFunction = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        // Wrap the task with monitoring
        auto wrappedTask = [this, taskName, threadIndex, taskStartTime, boundFunction = std::move(boundFunction)]()
        {
            try
            {
                boundFunction();
                UpdateThreadStats(threadIndex, taskName, taskStartTime);
                m_threadStats[threadIndex].tasksProcessed++;
            }
            catch (const std::exception &e)
            {
                m_threadStats[threadIndex].tasksFailed++;
                DEBUG_ERROR("Task failed in thread ", threadIndex, ": ", e.what());
                throw;
            }
        };

        m_threadStats[threadIndex].tasksQueued++;
        m_threadPool->enqueueTask(Task(std::move(wrappedTask)));
    }

    // Thread management
    void SetThreadPriority(size_t threadIndex, ThreadPriority priority);
    void AdjustThreadCount(size_t newCount);
    void PauseThread(size_t threadIndex);
    void ResumeThread(size_t threadIndex);

    // Monitoring and statistics
    const ThreadStats &GetThreadStats(size_t threadIndex) const;
    std::vector<ThreadStats> GetAllThreadStats() const;
    float GetThreadUtilization() const;
    size_t GetActiveThreadCount() const;

    // System control
    void Shutdown();
    void EmergencyStop();

private:
    void InitializeThreads(size_t numThreads);
    void MonitorThreadHealth();
    void UpdateThreadStats(size_t threadIndex, const std::string &taskName,
                           std::chrono::steady_clock::time_point startTime);

    std::unique_ptr<ThreadPool> m_threadPool;
    std::vector<ThreadStats> m_threadStats;
    std::mutex m_statsMutex;
    std::atomic<bool> m_isMonitoring;
    std::thread m_monitorThread;

    static constexpr size_t MAX_THREADS = 64;
    static constexpr std::chrono::seconds MONITOR_INTERVAL{1};
    static constexpr double HEALTH_CHECK_THRESHOLD = 0.9;
};