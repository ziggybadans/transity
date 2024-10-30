#include "WindowManager.h"

/**
<summary>
WindowManager is responsible for creating and managing the main application window.
It provides methods to initialize the window, set video properties, and handle rendering.
This class abstracts away the complexity of managing SFML's RenderWindow and offers
an easy-to-use interface for window-related operations.
</summary>
*/
WindowManager::WindowManager()
    : window(nullptr),
    videoMode(1280, 720), // Default resolution
    windowTitle("2D Transport Management Game"), // Default title
    contextSettings() {} // Default context settings

/**
<summary>
Destructor for WindowManager. Ensures that the window is properly closed
before the object is destroyed.
</summary>
*/
WindowManager::~WindowManager() {
    if (window && window->isOpen()) {
        window->close();
    }
}

/**
<summary>
Sets the video mode for the window.
</summary>
<param name="vm">The new video mode to be set (e.g., resolution).</param>
*/
void WindowManager::SetVideoMode(const sf::VideoMode& vm) {
    videoMode = vm;
}

/**
<summary>
Sets the title of the window.
</summary>
<param name="title">The new title for the window.</param>
*/
void WindowManager::SetTitle(const std::string& title) {
    windowTitle = title;
}

/**
<summary>
Sets the OpenGL context settings for the window.
</summary>
<param name="settings">The context settings to be applied (e.g., depth buffer bits).</param>
*/
void WindowManager::SetContextSettings(const sf::ContextSettings& settings) {
    contextSettings = settings;
}

/**
<summary>
Initializes the window with the specified settings, including video mode, title, and context.
It also sets the frame rate limit to 240 frames per second.
</summary>
<returns>True if the window was successfully created and is open, otherwise false.</returns>
*/
bool WindowManager::Init() {
    // Check if fullscreen is enabled and set appropriate style
    sf::Uint32 style = fullscreen ? sf::Style::Fullscreen : sf::Style::Default;

    window = std::make_unique<sf::RenderWindow>(videoMode, windowTitle, style, contextSettings);
    window->setFramerateLimit(240);

    return window->isOpen();
}

/**
<summary>
Polls for events from the window's event queue.
</summary>
<param name="event">Reference to an SFML Event object to be populated.</param>
<returns>True if an event was polled, otherwise false.</returns>
*/
bool WindowManager::PollEvent(sf::Event& event) {
    if (window) {
        return window->pollEvent(event);
    }
    return false;
}

/**
<summary>
Checks if the window is currently open.
</summary>
<returns>True if the window is open, otherwise false.</returns>
*/
bool WindowManager::IsOpen() const {
    return window && window->isOpen();
}

/**
<summary>
Clears the window with the specified color.
</summary>
<param name="color">The color to clear the window with.</param>
*/
void WindowManager::Clear(const sf::Color& color) {
    if (window) {
        window->clear(color);
    }
}

/**
<summary>
Displays the contents of the current render buffer on the window.
</summary>
*/
void WindowManager::Display() {
    if (window) {
        window->display();
    }
}

/**
<summary>
Provides access to the underlying SFML RenderWindow.
</summary>
<returns>A reference to the SFML RenderWindow object.</returns>
<exception cref="std::runtime_error">Thrown if the window is not initialized.</exception>
*/
sf::RenderWindow& WindowManager::GetWindow() {
    if (window) {
        return *window;
    }
    throw std::runtime_error("RenderWindow is not initialized.");
}

/**
<summary>
Enables or disables fullscreen mode for the window.
</summary>
<param name="enable">True to enable fullscreen, false to disable it.</param>
*/
void WindowManager::SetFullscreen(bool enable) {
    fullscreen = enable;
}
