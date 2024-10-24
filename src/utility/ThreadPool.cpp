// ThreadPool.cpp
#include "ThreadPool.h"
#include <stdexcept>
#include <iostream>

ThreadPool::ThreadPool(size_t numThreads) : m_stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::enqueueTask(Task task) { // Pass by value
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_stop.load()) {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }
        m_tasks.emplace(std::move(task));
    }
    m_condition.notify_one();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stop.store(true);
    }
    m_condition.notify_all();
    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerThread() {
    while (true) {
        Task task([] {}); // Default empty task
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_condition.wait(lock, [this]() { return m_stop.load() || !m_tasks.empty(); });
            if (m_stop.load() && m_tasks.empty()) {
                return;
            }
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        try {
            task.execute();
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in task: " << e.what() << std::endl;
            // Optionally handle specific exceptions
        }
        catch (...) {
            std::cerr << "Unknown exception in task." << std::endl;
        }
    }
}
