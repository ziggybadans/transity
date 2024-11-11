// Task.h
#pragma once
#include <functional>
#include <memory>

class Task {
public:
    using TaskFunc = std::function<void()>;

    // Constructor that takes a function and wraps it in a shared pointer.
    explicit Task(TaskFunc func) : m_func(std::make_shared<TaskFunc>(func)) {}

    // Executes the task if it is valid.
    void execute() const {
        if (m_func && *m_func) {
            (*m_func)();
        }
    }

private:
    // Shared pointer to the function, allowing for shared ownership and safe memory management.
    std::shared_ptr<TaskFunc> m_func;
};
