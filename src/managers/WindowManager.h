#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "InitializationManager.h"

class WindowManager : public IInitializable { // Changed to public inheritance
public:
    WindowManager();
    ~WindowManager();

    // Initialize the SFML window without parameters
    bool Init() override;

    // Set video mode before initialization
    void SetVideoMode(const sf::VideoMode& vm);

    // Set window title before initialization
    void SetTitle(const std::string& title);

    // Set context settings before initialization
    void SetContextSettings(const sf::ContextSettings& settings);

    // Poll and retrieve window events
    bool PollEvent(sf::Event& event);

    // Check if the window is open
    bool IsOpen() const;

    // Clear the window with a specified color
    void Clear(const sf::Color& color = sf::Color::Black);

    // Display the rendered frame
    void Display();

    // Get a reference to the SFML window
    sf::RenderWindow& GetWindow();

    // Enable or disable fullscreen mode
    void SetFullscreen(bool enable);

private:
    std::unique_ptr<sf::RenderWindow> window; // Pointer to the SFML window

    // Configuration parameters
    sf::VideoMode videoMode; // Video mode (resolution, color depth, etc.)
    std::string windowTitle; // Title of the window
    sf::ContextSettings contextSettings; // OpenGL context settings

    bool fullscreen; // Flag to indicate whether fullscreen is enabled
};
