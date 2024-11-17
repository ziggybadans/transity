#include "WindowManager.h"
#include "../Debug.h"

WindowManager::WindowManager()
    : m_window(nullptr),
      m_videoMode(1280, 720),      // Default resolution
      m_windowTitle("2D Transport Management Game"),
      m_contextSettings(),         // Default context settings
      m_fullscreen(false),
      m_vsyncEnabled(false),
      m_frameRateLimit(240)
{}

WindowManager::~WindowManager() {
    if (m_window && m_window->isOpen()) {
        m_window->close();
    }
}

bool WindowManager::Init() {
    sf::Uint32 style = m_fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
    
    try {
        m_window = std::make_unique<sf::RenderWindow>(
            m_videoMode, 
            m_windowTitle, 
            style, 
            m_contextSettings
        );
        m_window->setFramerateLimit(240);
        return m_window->isOpen();
    } catch (const std::exception& e) {
        DEBUG_ERROR("Failed to initialize window: ", e.what());
        return false;
    }
}

bool WindowManager::PollEvent(sf::Event& event) {
    return m_window && m_window->pollEvent(event);
}

void WindowManager::Clear(const sf::Color& color) {
    if (m_window) {
        m_window->clear(color);
    }
}

void WindowManager::Display() {
    if (m_window) {
        m_window->display();
    }
}

void WindowManager::SetVideoMode(const sf::VideoMode& vm) {
    m_videoMode = vm;
}

void WindowManager::SetTitle(const std::string& title) {
    m_windowTitle = title;
}

void WindowManager::SetContextSettings(const sf::ContextSettings& settings) {
    m_contextSettings = settings;
}

void WindowManager::SetFullscreen(bool enable) {
    m_fullscreen = enable;
}

bool WindowManager::IsOpen() const {
    return m_window && m_window->isOpen();
}

sf::RenderWindow& WindowManager::GetWindow() {
    if (!m_window) {
        throw std::runtime_error("Attempting to access uninitialized window");
    }
    return *m_window;
}

void WindowManager::ApplyVideoMode() {
    if (!m_window) return;
    
    sf::Uint32 style = m_fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
    m_window->create(m_videoMode, m_windowTitle, style, m_contextSettings);
    m_window->setVerticalSyncEnabled(m_vsyncEnabled);
    m_window->setFramerateLimit(m_frameRateLimit);
}

void WindowManager::SetVSync(bool enabled) {
    m_vsyncEnabled = enabled;
    if (m_window) {
        m_window->setVerticalSyncEnabled(enabled);
    }
}

void WindowManager::SetFramerateLimit(unsigned int limit) {
    m_frameRateLimit = limit;
    if (m_window) {
        m_window->setFramerateLimit(limit);
    }
}
