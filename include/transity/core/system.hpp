#pragma once

#include <string>
#include <memory>

namespace transity {
namespace core {

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

protected:
    bool m_enabled{true};
    int m_priority{0};
};

} // namespace core
} // namespace transity 