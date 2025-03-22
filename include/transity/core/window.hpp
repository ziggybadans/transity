#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <string>
#include <memory>

namespace transity {
namespace core {

/**
 * @brief Configuration struct for window settings
 */
struct WindowConfig {
    unsigned int width = 1280;
    unsigned int height = 720;
    std::string title = "Transity";
    bool fullscreen = false;
    unsigned int framerate = 60;
};

/**
 * @brief Window management class wrapping SFML window functionality
 * 
 * This class handles window creation, configuration, and event processing.
 */
class Window {
public:
    /**
     * @brief Construct a new Window with given configuration
     * @param config Window configuration settings
     */
    explicit Window(const WindowConfig& config = WindowConfig{});

    /**
     * @brief Destroy the Window and cleanup resources
     */
    ~Window();

    /**
     * @brief Create or recreate the window with current configuration
     */
    void create();

    /**
     * @brief Process all pending window events
     * @return true if window should remain open, false if close requested
     */
    bool processEvents();

    /**
     * @brief Begin frame rendering
     */
    void beginFrame();

    /**
     * @brief End frame rendering and display
     */
    void endFrame();

    /**
     * @brief Check if window is open
     * @return true if window is open, false otherwise
     */
    bool isOpen() const;

    /**
     * @brief Get the underlying SFML window
     * @return Reference to the SFML window
     */
    sf::RenderWindow& getWindow() { return *m_window; }
    const sf::RenderWindow& getWindow() const { return *m_window; }

    /**
     * @brief Get current window configuration
     * @return const reference to window configuration
     */
    const WindowConfig& getConfig() const { return m_config; }

    /**
     * @brief Update window configuration and recreate if necessary
     * @param config New window configuration
     */
    void setConfig(const WindowConfig& config);

private:
    std::unique_ptr<sf::RenderWindow> m_window;
    WindowConfig m_config;
};

} // namespace core
} // namespace transity 