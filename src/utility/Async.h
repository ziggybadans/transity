// Async.h
#pragma once
#include "ThreadPool.h"
#include <future>

class Async {
public:
    Async(ThreadPool& pool) : m_pool(pool) {}

    template <typename Func, typename... Args>
    auto run(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using ReturnType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        std::future<ReturnType> res = task->get_future();
        m_pool.enqueueTask(Task([task]() { (*task)(); }));
        return res;
    }

private:
    ThreadPool& m_pool;
};
