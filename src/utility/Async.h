// Async.h
#pragma once
#include "ThreadPool.h"
#include <future>

class Async {
public:
    // Constructor that initializes the Async instance with a reference to a ThreadPool.
    Async(ThreadPool& pool) : m_pool(pool) {}

    // Runs a given function asynchronously using the thread pool.
    template <typename Func, typename... Args>
    auto run(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using ReturnType = decltype(func(args...));
        // Create a packaged_task that wraps the function and its arguments.
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        // Get the future result of the task.
        std::future<ReturnType> res = task->get_future();
        // Enqueue the task in the thread pool for execution.
        m_pool.enqueueTask(Task([task]() { (*task)(); }));
        return res; // Return the future to allow the caller to retrieve the result.
    }

private:
    ThreadPool& m_pool; // Reference to the thread pool used for executing tasks.
};
