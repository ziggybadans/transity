#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "transity/core/input_manager.hpp"
#include "transity/core/system_manager.hpp"
#include "transity/core/window.hpp"

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
     * @brief Run the main application loop
     * @throws ApplicationError if application is not initialized
     */
    void run();

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

    /**
     * @brief Game states enum
     */
    enum class GameState {
        Running,    ///< Game is running normally
        Paused,     ///< Game is paused
        Stopped     ///< Game is stopped
    };

    /**
     * @brief Get current game state
     * @return Current GameState
     */
    GameState getGameState() const { return m_gameState; }

    /**
     * @brief Pause the game
     */
    void pause();

    /**
     * @brief Resume the game
     */
    void resume();

    /**
     * @brief Stop the game
     */
    void stop();

    /**
     * @brief Set target FPS for the game loop
     * @param fps Target frames per second (0 for unlimited)
     */
    void setTargetFPS(unsigned int fps) { m_targetFPS = fps; }

    /**
     * @brief Get current FPS
     * @return Current frames per second
     */
    float getCurrentFPS() const { return m_currentFPS; }

    /**
     * @brief Process an SFML event
     * @param event The event to process
     */
    void processEvent(const sf::Event& event);

    /**
     * @brief Get the system manager instance
     * @return Reference to the SystemManager
     */
    SystemManager& getSystemManager() { return m_systemManager; }

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

private:
    Application(); // Private constructor for singleton pattern
    ~Application(); // Private destructor

    bool m_initialized;
    std::string m_appName;
    SystemManager m_systemManager;
    sf::Clock m_clock;        // SFML clock for timing
    sf::Clock m_fpsTimer;     // Separate clock for FPS calculation
    std::unique_ptr<Window> m_window; // Main application window

    /**
     * @brief Update game logic with fixed timestep
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Render game state
     */
    void render();

    GameState m_gameState;
    unsigned int m_targetFPS;
    float m_currentFPS;
    float m_accumulatedTime;
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f; // 60 updates per second

    void processInput();  // New method for input processing
};

} // namespace core
} // namespace transity 