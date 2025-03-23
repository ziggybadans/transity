#include "transity/core/window_manager.hpp"
#include "transity/core/debug_manager.hpp"

namespace transity::core {

WindowManager& WindowManager::getInstance() {
    static WindowManager instance;
    return instance;
}

Window& WindowManager::createWindow(const std::string& windowId, const WindowConfig& config) {
    auto& debug = DebugManager::getInstance();
    debug.beginMetric("window_creation", "core", "ms");

    if (hasWindow(windowId)) {
        debug.log(LogLevel::Error, "Window creation failed: ID '" + windowId + "' already exists");
        debug.endMetric("window_creation");
        throw ConfigurationError("Window with ID '" + windowId + "' already exists");
    }

    try {
        // Validate window ID
        if (windowId.empty()) {
            throw ConfigurationError("Window ID cannot be empty");
        }

        // Create the window
        auto window = std::make_unique<Window>(config);
        
        // Verify window was created successfully
        if (!window || !window->isOpen()) {
            throw ConfigurationError("Window creation failed: invalid window state");
        }

        // Store the window
        auto [it, success] = m_windows.emplace(windowId, std::move(window));
        if (!success) {
            throw ConfigurationError("Failed to store window in manager");
        }

        debug.log(LogLevel::Info, "Created window with ID: " + windowId);
        debug.endMetric("window_creation");
        return *it->second;
    } catch (const std::exception& e) {
        debug.log(LogLevel::Error, "Window creation failed: " + std::string(e.what()));
        debug.endMetric("window_creation");
        throw ConfigurationError("Failed to create window: " + std::string(e.what()));
    }
}

Window& WindowManager::getWindow(const std::string& windowId) {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
        throw ConfigurationError("Window with ID '" + windowId + "' does not exist");
    }
    return *it->second;
}

const Window& WindowManager::getWindow(const std::string& windowId) const {
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) {
        throw ConfigurationError("Window with ID '" + windowId + "' does not exist");
    }
    return *it->second;
}

bool WindowManager::hasWindow(const std::string& windowId) const {
    return m_windows.find(windowId) != m_windows.end();
}

void WindowManager::destroyWindow(const std::string& windowId) {
    auto& debug = DebugManager::getInstance();
    debug.beginMetric("window_destruction", "core", "ms");

    if (!hasWindow(windowId)) {
        throw ConfigurationError("Cannot destroy window: ID '" + windowId + "' does not exist");
    }

    m_windows.erase(windowId);
    debug.log(LogLevel::Info, "Destroyed window with ID: " + windowId);
    debug.endMetric("window_destruction");
}

void WindowManager::cleanup() {
    auto& debug = DebugManager::getInstance();
    debug.beginMetric("window_cleanup", "core", "ms");

    m_windows.clear();
    debug.log(LogLevel::Info, "All windows destroyed");
    debug.endMetric("window_cleanup");
}

bool WindowManager::processEvents() {
    bool anyWindowOpen = false;
    std::vector<std::string> windowsToClose;

    for (const auto& [id, window] : m_windows) {
        if (window->isOpen()) {
            if (!window->processEvents()) {
                windowsToClose.push_back(id);
            } else {
                anyWindowOpen = true;
            }
        }
    }

    // Clean up windows that requested to close
    for (const auto& id : windowsToClose) {
        destroyWindow(id);
    }

    return anyWindowOpen;
}

void WindowManager::beginFrame() {
    for (const auto& [id, window] : m_windows) {
        if (window->isOpen()) {
            window->beginFrame();
        }
    }
}

void WindowManager::endFrame() {
    for (const auto& [id, window] : m_windows) {
        if (window->isOpen()) {
            window->endFrame();
        }
    }
}

bool WindowManager::hasOpenWindows() const {
    for (const auto& [id, window] : m_windows) {
        if (window->isOpen()) {
            return true;
        }
    }
    return false;
}

} // namespace transity::core 