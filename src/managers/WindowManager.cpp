#include "WindowManager.h"

WindowManager::WindowManager()
    : window(nullptr),
    videoMode(1280, 720), // Default resolution
    windowTitle("2D Transport Management Game"), // Default title
    contextSettings() {} // Default context settings

WindowManager::~WindowManager() {
    if (window && window->isOpen()) {
        window->close();
    }
}

void WindowManager::SetVideoMode(const sf::VideoMode& vm) {
    videoMode = vm;
}

void WindowManager::SetTitle(const std::string& title) {
    windowTitle = title;
}

void WindowManager::SetContextSettings(const sf::ContextSettings& settings) {
    contextSettings = settings;
}

bool WindowManager::Init() {
    window = std::make_unique<sf::RenderWindow>(videoMode, windowTitle, sf::Style::Default, contextSettings);
    window->setFramerateLimit(60);

    return window->isOpen();
}

bool WindowManager::PollEvent(sf::Event& event) {
    if (window) {
        return window->pollEvent(event);
    }
    return false;
}

bool WindowManager::IsOpen() const {
    return window && window->isOpen();
}

void WindowManager::Clear(const sf::Color& color) {
    if (window) {
        window->clear(color);
    }
}

void WindowManager::Display() {
    if (window) {
        window->display();
    }
}

sf::RenderWindow& WindowManager::GetWindow() {
    if (window) {
        return *window;
    }
    throw std::runtime_error("RenderWindow is not initialized.");
}