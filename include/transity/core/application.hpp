#pragma once

#include <memory>
#include <string>
#include <stdexcept>

namespace transity {
namespace core {

/**
 * @brief Custom exception class for Application-related errors
 */
class ApplicationError : public std::runtime_error {
public:
    explicit ApplicationError(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Main application class implementing the singleton pattern
 * 
 * This class serves as the core of the Transity application, managing the application
 * lifecycle and providing access to core systems.
 */
class Application {
public:
    /**
     * @brief Get the singleton instance of the Application
     * @return Reference to the Application instance
     */
    static Application& getInstance();

    /**
     * @brief Initialize the application with given parameters
     * @param appName Name of the application
     * @throws ApplicationError if initialization fails
     */
    void initialize(const std::string& appName = "Transity");

    /**
     * @brief Shutdown the application and cleanup resources
     */
    void shutdown();

    /**
     * @brief Check if the application is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Get the application name
     * @return const reference to the application name
     */
    const std::string& getAppName() const { return m_appName; }

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

private:
    Application(); // Private constructor for singleton pattern
    ~Application(); // Private destructor

    bool m_initialized;
    std::string m_appName;
};

} // namespace core
} // namespace transity 