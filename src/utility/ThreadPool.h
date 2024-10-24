// ThreadPool.h
#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "Task.h"

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    // Prevent copying
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void enqueueTask(Task task);
    void shutdown();

private:
    void workerThread();

    std::vector<std::thread> m_workers;
    std::queue<Task> m_tasks;

    std::mutex m_queueMutex; // Use std::mutex directly
    std::condition_variable m_condition;
    std::atomic<bool> m_stop;
};
