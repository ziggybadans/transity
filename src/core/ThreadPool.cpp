#include "ThreadPool.h"

#include <stdexcept>
#include <iostream>

// Constructor that initializes the thread pool with a given number of threads.
ThreadPool::ThreadPool(size_t numThreads) : m_stop(false) {
    try {
        // Create worker threads and add them to the m_workers vector.
        for (size_t i = 0; i < numThreads; ++i) {
            // Each thread runs the workerThread function, with 'this' as the context for accessing the class members.
            m_workers.emplace_back(&ThreadPool::workerThread, this);
        }
    } catch (...) {
        shutdown(); // Clean up any threads that were created
        throw;      // Re-throw the exception
    }
}

// Destructor that ensures proper cleanup of threads and resources.
ThreadPool::~ThreadPool() {
    shutdown();
}

// Adds a task to the queue for the thread pool to execute.
void ThreadPool::enqueueTask(Task task) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_stop.load()) {
            throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
        }
        m_tasks.emplace(std::move(task));
    }
    m_condition.notify_one();
}

// Shuts down the thread pool gracefully.
void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stop.store(true);
    }
    m_condition.notify_all();

    // Join all worker threads
    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            try {
                worker.join();
            } catch (const std::exception& e) {
                std::cerr << "Error joining thread: " << e.what() << std::endl;
            }
        }
    }
    
    // Clear the workers vector
    m_workers.clear();
    
    // Clear any remaining tasks
    std::queue<Task> empty;
    std::swap(m_tasks, empty);
}

// Function executed by each worker thread.
void ThreadPool::workerThread() {
    while (true) {
        std::unique_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_condition.wait(lock, [this]() { 
                return m_stop.load() || !m_tasks.empty(); 
            });
            
            if (m_stop.load() && m_tasks.empty()) {
                return;
            }
            
            task = std::make_unique<Task>(std::move(m_tasks.front()));
            m_tasks.pop();
        }
        
        try {
            if (task) {
                task->execute();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in task: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in task." << std::endl;
        }
    }
}
