// Task.h
#pragma once
#include <functional>
#include <memory>

class Task {
public:
    using TaskFunc = std::function<void()>;

    explicit Task(TaskFunc func) : m_func(std::make_shared<TaskFunc>(func)) {}

    void execute() const {
        if (m_func && *m_func) {
            (*m_func)();
        }
    }

private:
    std::shared_ptr<TaskFunc> m_func;
};
