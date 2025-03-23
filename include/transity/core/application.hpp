#pragma once

#include <memory>
#include <string>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "transity/core/input_manager.hpp"
#include "transity/core/system_manager.hpp"
#include "transity/core/window.hpp"
#include "transity/core/error.hpp"
#include "transity/core/debug_manager.hpp"

namespace transity {
namespace core {

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
     * @throws InitializationError if initialization fails
     */
    void initialize(const std::string& appName = "Transity");

    /**
     * @brief Shutdown the application and cleanup resources
     * @throws SystemError if shutdown fails
     */
    void shutdown();

    /**
     * @brief Run the main application loop
     * @throws StateError if application is not initialized
     * @throws RuntimeError if a critical error occurs during execution
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
     * @throws StateError if game is not in a pausable state
     */
    void pause();

    /**
     * @brief Resume the game
     * @throws StateError if game is not paused
     */
    void resume();

    /**
     * @brief Stop the game
     */
    void stop();

    /**
     * @brief Set target FPS for the game loop
     * @param fps Target frames per second (0 for unlimited)
     * @throws StateError if fps value is invalid
     */
    void setTargetFPS(unsigned int fps);

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
     * @throws RuntimeError if update fails
     */
    void update(float deltaTime);

    /**
     * @brief Render game state
     * @throws RuntimeError if rendering fails
     */
    void render();

    GameState m_gameState;
    unsigned int m_targetFPS;
    float m_currentFPS;
    float m_accumulatedTime;
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f; // 60 updates per second
    static constexpr unsigned int MAX_FPS = 1000; // Maximum allowed FPS

    void processInput();  // New method for input processing

    /**
     * @brief Update performance metrics
     */
    void updatePerformanceMetrics();
};

} // namespace core
} // namespace transity 