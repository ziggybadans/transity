#include "transity/core/window.hpp"
#include <SFML/Window/Event.hpp>

namespace transity {
namespace core {

Window::Window(const WindowConfig& config)
    : m_config(config) {
    create();
}

Window::~Window() = default;

void Window::create() {
    // Set up window style based on configuration
    sf::Uint32 style = sf::Style::Default;
    if (m_config.fullscreen) {
        style = sf::Style::Fullscreen;
    }

    // Create the SFML window
    m_window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(m_config.width, m_config.height),
        m_config.title,
        style
    );

    // Apply framerate limit
    m_window->setFramerateLimit(m_config.framerate);
}

bool Window::processEvents() {
    sf::Event event;
    while (m_window->pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                m_window->close();
                return false;
            case sf::Event::Resized:
                // Update the view to match the new window size
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                m_window->setView(sf::View(visibleArea));
                break;
            default:
                break;
        }
    }
    return true;
}

void Window::beginFrame() {
    m_window->clear(sf::Color::Black);
}

void Window::endFrame() {
    m_window->display();
}

bool Window::isOpen() const {
    return m_window && m_window->isOpen();
}

void Window::setConfig(const WindowConfig& config) {
    bool needsRecreate = m_config.width != config.width ||
                        m_config.height != config.height ||
                        m_config.fullscreen != config.fullscreen;

    m_config = config;

    if (needsRecreate) {
        create();
    } else {
        m_window->setFramerateLimit(m_config.framerate);
        m_window->setTitle(m_config.title);
    }
}

} // namespace core
} // namespace transity 