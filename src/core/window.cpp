#include "transity/core/window.hpp"
#include "transity/core/error.hpp"
#include "transity/core/application.hpp"
#include <SFML/Window/Event.hpp>

namespace transity {
namespace core {

Window::Window(const WindowConfig& config)
    : m_config(config) {
    create();
}

Window::~Window() = default;

void Window::create() {
    // Validate window configuration
    if (m_config.width == 0 || m_config.height == 0) {
        throw ConfigurationError("Invalid window dimensions");
    }

    // In CI environments, use a more conservative video mode
    sf::VideoMode videoMode;
    if (std::getenv("CI") != nullptr) {
        // Use a smaller, more compatible resolution in CI
        videoMode = sf::VideoMode(800, 600, 24);
    } else {
        videoMode = sf::VideoMode(m_config.width, m_config.height);
    }

    if (!videoMode.isValid()) {
        throw ConfigurationError("Invalid video mode: " + std::to_string(videoMode.width) + "x" + std::to_string(videoMode.height));
    }

    // Set up window style based on configuration
    sf::Uint32 style = sf::Style::Default;
    if (m_config.fullscreen) {
        if (!sf::VideoMode::getFullscreenModes().empty()) {
            videoMode = sf::VideoMode::getFullscreenModes()[0];
            style = sf::Style::Fullscreen;
        } else {
            throw ConfigurationError("No valid fullscreen video modes available");
        }
    }

    try {
        // Create the SFML window
        m_window = std::make_unique<sf::RenderWindow>(
            videoMode,
            m_config.title,
            style
        );

        if (!m_window || !m_window->isOpen()) {
            throw ConfigurationError("Failed to create window");
        }

        // Apply framerate limit
        m_window->setFramerateLimit(m_config.framerate);
    } catch (const std::exception& e) {
        throw ConfigurationError(std::string("Window creation failed: ") + e.what());
    }
}

bool Window::processEvents() {
    if (!m_window || !m_window->isOpen()) {
        return false;
    }

    sf::Event event;
    while (m_window->pollEvent(event)) {
        // First let the application handle the event
        Application::getInstance().processEvent(event);

        // Then handle window-specific events
        switch (event.type) {
            case sf::Event::Closed:
                m_window->close();
                return false;
            case sf::Event::Resized: {
                // Update the view to match the new window size
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                m_window->setView(sf::View(visibleArea));
                break;
            }
            case sf::Event::LostFocus: {
                // Optionally pause the game when window loses focus
                auto& app = Application::getInstance();
                if (app.getGameState() == Application::GameState::Running) {
                    app.pause();
                }
                break;
            }
            case sf::Event::GainedFocus: {
                // Optionally resume the game when window gains focus
                auto& app = Application::getInstance();
                if (app.getGameState() == Application::GameState::Paused) {
                    app.resume();
                }
                break;
            }
            default:
                break;
        }
    }

    return m_window->isOpen();
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