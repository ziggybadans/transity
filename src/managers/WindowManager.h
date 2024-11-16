#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "InitializationManager.h"

class WindowManager : public IInitializable {
public:
    WindowManager();
    ~WindowManager();

    /* Virtual Methods */
    bool Init() override;

    /* Window Operations */
    bool PollEvent(sf::Event& event);
    void Clear(const sf::Color& color = sf::Color::Black);
    void Display();

    /* Setters */
    void SetVideoMode(const sf::VideoMode& vm);
    void SetTitle(const std::string& title);
    void SetContextSettings(const sf::ContextSettings& settings);
    void SetFullscreen(bool enable);

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
};
