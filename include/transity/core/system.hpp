#pragma once

#include <string>
#include <memory>
#include <future>
#include <functional>

namespace transity {
namespace core {

// Forward declaration for ResourceHandle
template<typename T>
class ResourceHandle;

/**
 * @brief Interface for all subsystems in the engine
 * 
 * Provides the base functionality that all subsystems must implement
 * including initialization, update, and shutdown hooks.
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /**
     * @brief Initialize the system
     * @return true if initialization succeeded, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * @brief Update the system
     * @param deltaTime Time elapsed since last update in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Shut down the system and cleanup resources
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get the name of the system
     * @return System name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Check if the system is enabled
     * @return true if enabled, false otherwise
     */
    bool isEnabled() const { return m_enabled; }

    /**
     * @brief Enable or disable the system
     * @param enabled New enabled state
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * @brief Get the update priority of the system
     * @return Update priority (higher numbers update later)
     */
    int getPriority() const { return m_priority; }

    /**
     * @brief Set the update priority of the system
     * @param priority New priority value
     */
    void setPriority(int priority) { m_priority = priority; }

    /**
     * @brief Asynchronously initialize the system
     * @return A future that will be resolved when initialization completes
     */
    std::future<bool> initializeAsync() {
        return std::async(std::launch::async, [this]() {
            return this->initialize();
        });
    }

    /**
     * @brief Check if this system is loading asynchronously
     * @return true if async loading is in progress
     */
    bool isLoadingAsync() const { return m_asyncLoading; }

    /**
     * @brief Set the async loading state
     * @param loading New loading state
     */
    void setAsyncLoading(bool loading) { m_asyncLoading = loading; }

    /**
     * @brief Schedule a task to run asynchronously
     * @param task Function to execute asynchronously
     * @return Future for the task result
     */
    template<typename Func>
    auto scheduleAsyncTask(Func&& task) -> std::future<decltype(task())> {
        return std::async(std::launch::async, std::forward<Func>(task));
    }

protected:
    bool m_enabled{true};
    int m_priority{0};
    bool m_asyncLoading{false};
};

/**
 * @brief Resource handle pattern for asset management
 * 
 * Provides a safe way to reference resources without direct pointers.
 * Handles can be copied and passed around while the underlying resource
 * is managed by a resource system.
 */
template<typename T>
class ResourceHandle {
public:
    ResourceHandle() : m_id(0), m_resource(nullptr) {}
    
    ResourceHandle(uint32_t id, T* resource) 
        : m_id(id), m_resource(resource) {}
    
    bool isValid() const { return m_resource != nullptr; }
    
    T* get() const { return m_resource; }
    
    uint32_t getId() const { return m_id; }
    
    T* operator->() const { return m_resource; }
    
    T& operator*() const { return *m_resource; }
    
    operator bool() const { return isValid(); }

private:
    uint32_t m_id;
    T* m_resource;
};

} // namespace core
} // namespace transity 