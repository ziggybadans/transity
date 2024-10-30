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
    // Constructor that initializes the thread pool with a specified number of threads.
    explicit ThreadPool(size_t numThreads);

    // Destructor that ensures proper cleanup of threads and resources.
    ~ThreadPool();

    // Prevent copying of the ThreadPool instance to avoid issues with multiple threads operating on the same data.
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Adds a task to the task queue for the thread pool to execute.
    void enqueueTask(Task task);

    // Shuts down the thread pool gracefully, ensuring all tasks are completed and threads are joined.
    void shutdown();

private:
    // Function run by each worker thread. Continuously fetches and executes tasks from the queue.
    void workerThread();

    // Vector that holds all the worker threads.
    std::vector<std::thread> m_workers;

    // Queue that holds the tasks waiting to be executed by the worker threads.
    std::queue<Task> m_tasks;

    // Mutex used to synchronize access to the task queue to ensure that multiple threads do not modify it simultaneously.
    std::mutex m_queueMutex;

    // Condition variable used to notify worker threads when new tasks are added to the queue or when shutdown is initiated.
    std::condition_variable m_condition;

    // Atomic boolean flag used to indicate whether the thread pool should stop processing tasks (used for graceful shutdown).
    std::atomic<bool> m_stop;
};
