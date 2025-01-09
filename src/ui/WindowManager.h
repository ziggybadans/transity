#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    bool Init();

    /* Window Operations */
    bool PollEvent(sf::Event& event);
    void Clear(const sf::Color& color = sf::Color::Black);
    void Display();
    void ApplyVideoMode();

    /* Setters */
    void SetVideoMode(const sf::VideoMode& vm);
    void SetTitle(const std::string& title);
    void SetContextSettings(const sf::ContextSettings& settings);
    void SetFullscreen(bool enable);
    void SetVSync(bool enabled);
    void SetFramerateLimit(unsigned int limit);

    /* Getters */
    bool IsOpen() const;
    sf::RenderWindow& GetWindow();

private:
    /* Window Properties */
    std::unique_ptr<sf::RenderWindow> m_window;
    sf::VideoMode m_videoMode;
    std::string m_windowTitle;
    sf::ContextSettings m_contextSettings;
    bool m_fullscreen;
    bool m_vsyncEnabled;
    unsigned int m_frameRateLimit;
};
