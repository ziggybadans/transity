// ThreadPool.cpp
#include "ThreadPool.h"
#include <stdexcept>
#include <iostream>

// Constructor that initializes the thread pool with a given number of threads.
ThreadPool::ThreadPool(size_t numThreads) : m_stop(false) {
    // Create worker threads and add them to the m_workers vector.
    for (size_t i = 0; i < numThreads; ++i) {
        // Each thread runs the workerThread function, with 'this' as the context for accessing the class members.
        m_workers.emplace_back(&ThreadPool::workerThread, this);
    }
}

// Destructor that ensures proper cleanup of threads and resources.
ThreadPool::~ThreadPool() {
    // Ensure a proper shutdown of the thread pool before destruction.
    shutdown();
}

// Adds a task to the queue for the thread pool to execute.
void ThreadPool::enqueueTask(Task task) { // Pass by value
    {
        // Lock the queue mutex to ensure that only one thread modifies the task queue at a time.
        std::lock_guard<std::mutex> lock(m_queueMutex);
        // If the thread pool is stopped, do not allow new tasks to be enqueued.
        if (m_stop.load()) {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }
        // Add the task to the queue.
        m_tasks.emplace(std::move(task));
    }
    // Notify one of the worker threads that a new task is available for processing.
    m_condition.notify_one();
}

// Shuts down the thread pool gracefully.
void ThreadPool::shutdown() {
    {
        // Lock the queue mutex to ensure thread-safe modification of the stop flag.
        std::lock_guard<std::mutex> lock(m_queueMutex);
        // Set the stop flag to true, indicating that no more tasks should be processed.
        m_stop.store(true);
    }
    // Notify all worker threads to wake up so they can exit if the stop flag is set.
    m_condition.notify_all();
    // Join all worker threads to ensure they have completed their tasks before shutting down.
    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// Function executed by each worker thread.
void ThreadPool::workerThread() {
    while (true) {
        Task task([] {}); // Default empty task to initialize the task variable.
        {
            // Lock the queue mutex to safely check for available tasks or a stop signal.
            std::unique_lock<std::mutex> lock(m_queueMutex);
            // Wait until there is a task available or the stop flag is set.
            m_condition.wait(lock, [this]() { return m_stop.load() || !m_tasks.empty(); });
            // If the stop flag is set and there are no tasks left, exit the thread.
            if (m_stop.load() && m_tasks.empty()) {
                return;
            }
            // Fetch the next task from the queue.
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        // Execute the fetched task.
        try {
            task.execute();
        }
        catch (const std::exception& e) {
            // Catch any exceptions thrown by the task and log an error message.
            std::cerr << "Exception in task: " << e.what() << std::endl;
        }
        catch (...) {
            // Catch any unknown exceptions and log an error message.
            std::cerr << "Unknown exception in task." << std::endl;
        }
    }
}
