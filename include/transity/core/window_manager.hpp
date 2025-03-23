#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "transity/core/window.hpp"
#include "transity/core/error.hpp"

namespace transity {
namespace core {

/**
 * @brief Manager class for handling multiple windows
 * 
 * This class manages creation, destruction, and access to multiple windows
 * in the application. Each window is identified by a unique string ID.
 */
class WindowManager {
public:
    /**
     * @brief Get the singleton instance of WindowManager
     * @return Reference to the WindowManager instance
     */
    static WindowManager& getInstance();

    /**
     * @brief Create a new window with given configuration
     * @param windowId Unique identifier for the window
     * @param config Window configuration settings
     * @throws ConfigurationError if window ID already exists
     * @return Reference to the created window
     */
    Window& createWindow(const std::string& windowId, const WindowConfig& config = WindowConfig{});

    /**
     * @brief Get a window by its ID
     * @param windowId The ID of the window to retrieve
     * @throws ConfigurationError if window ID doesn't exist
     * @return Reference to the window
     */
    Window& getWindow(const std::string& windowId);
    const Window& getWindow(const std::string& windowId) const;

    /**
     * @brief Check if a window with given ID exists
     * @param windowId The ID to check
     * @return true if window exists, false otherwise
     */
    bool hasWindow(const std::string& windowId) const;

    /**
     * @brief Close and destroy a window
     * @param windowId The ID of the window to destroy
     * @throws ConfigurationError if window ID doesn't exist
     */
    void destroyWindow(const std::string& windowId);

    /**
     * @brief Process events for all windows
     * @return true if any window should remain open, false if all windows requested close
     */
    bool processEvents();

    /**
     * @brief Begin frame rendering for all windows
     */
    void beginFrame();

    /**
     * @brief End frame rendering and display all windows
     */
    void endFrame();

    /**
     * @brief Check if any windows are open
     * @return true if any window is open, false otherwise
     */
    bool hasOpenWindows() const;

    /**
     * @brief Get the number of currently managed windows
     * @return Number of windows
     */
    size_t getWindowCount() const { return m_windows.size(); }

    /**
     * @brief Close and destroy all windows
     */
    void cleanup();

    // Delete copy constructor and assignment operator
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

private:
    WindowManager() = default;  // Private constructor for singleton pattern
    ~WindowManager() = default;

    std::unordered_map<std::string, std::unique_ptr<Window>> m_windows;
};

} // namespace core
} // namespace transity 