#include "transity/core/application.hpp"
#include <iostream>
#include <stdexcept>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>

namespace transity::core {

Application::Application()
    : m_initialized(false)
    , m_appName("")
    , m_gameState(GameState::Stopped)
    , m_targetFPS(60)
    , m_currentFPS(0.0f)
    , m_accumulatedTime(0.0f)
{
}

Application::~Application() {
    if (m_initialized) {
        shutdown();
    }
}

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

void Application::initialize(const std::string& appName) {
    if (m_initialized) {
        throw ApplicationError("Application is already initialized");
    }

    try {
        m_appName = appName;
        m_gameState = GameState::Running;
        
        // TODO: Initialize subsystems here
        
        m_initialized = true;
        std::cout << "Application '" << m_appName << "' initialized successfully" << std::endl;
    }
    catch (const std::exception& e) {
        throw ApplicationError(std::string("Failed to initialize application: ") + e.what());
    }
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }

    try {
        // TODO: Shutdown subsystems here in reverse order
        
        m_initialized = false;
        std::cout << "Application '" << m_appName << "' shut down successfully" << std::endl;
    }
    catch (const std::exception& e) {
        // Log error but don't throw during shutdown
        std::cerr << "Error during shutdown: " << e.what() << std::endl;
    }
}

void Application::run() {
    if (!m_initialized) {
        throw ApplicationError("Cannot run application before initialization");
    }

    sf::Clock clock;
    sf::Clock fpsTimer;
    unsigned int frameCount = 0;

    while (m_gameState != GameState::Stopped) {
        float deltaTime = clock.restart().asSeconds();
        m_accumulatedTime += deltaTime;

        // Update game logic with fixed timestep
        if (m_gameState == GameState::Running) {
            while (m_accumulatedTime >= FIXED_TIMESTEP) {
                update(FIXED_TIMESTEP);
                m_accumulatedTime -= FIXED_TIMESTEP;
            }
        }

        // Render at the target frame rate
        render();
        frameCount++;

        // Calculate FPS every second
        if (fpsTimer.getElapsedTime().asSeconds() >= 1.0f) {
            m_currentFPS = static_cast<float>(frameCount);
            frameCount = 0;
            fpsTimer.restart();
        }

        // Frame rate limiting
        if (m_targetFPS > 0) {
            float targetFrameTime = 1.0f / static_cast<float>(m_targetFPS);
            float elapsedTime = clock.getElapsedTime().asSeconds();
            if (elapsedTime < targetFrameTime) {
                sf::sleep(sf::seconds(targetFrameTime - elapsedTime));
            }
        }
    }
}

void Application::processEvent(const sf::Event& event) {
    // Let the input manager process the event
    InputManager::getInstance().processEvent(event);

    // Handle application-specific events
    if (event.type == sf::Event::Closed) {
        stop();
    }
}

void Application::processInput() {
    // Update input manager state
    InputManager::getInstance().update();

    // Handle any input-based game logic here
    if (InputManager::getInstance().isKeyJustPressed(sf::Keyboard::Escape)) {
        if (m_gameState == GameState::Running) {
            pause();
        } else if (m_gameState == GameState::Paused) {
            resume();
        }
    }
}

void Application::update(float deltaTime) {
    processInput();
    // TODO: Update other game systems
}

void Application::render() {
    // TODO: Render game state
    
    // Clear input states for next frame
    InputManager::getInstance().clear();
}

void Application::pause() {
    if (m_gameState == GameState::Running) {
        m_gameState = GameState::Paused;
        std::cout << "Game paused" << std::endl;
    }
}

void Application::resume() {
    if (m_gameState == GameState::Paused) {
        m_gameState = GameState::Running;
        std::cout << "Game resumed" << std::endl;
    }
}

void Application::stop() {
    m_gameState = GameState::Stopped;
    std::cout << "Game stopped" << std::endl;
}

} // namespace transity::core 