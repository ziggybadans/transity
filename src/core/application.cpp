#include "transity/core/application.hpp"
#include <iostream>
#include <stdexcept>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace transity::core {

Application::Application()
    : m_initialized(false)
    , m_appName("")
    , m_gameState(GameState::Stopped)
    , m_targetFPS(60)
    , m_currentFPS(0.0f)
    , m_accumulatedTime(0.0f)
    , m_window(nullptr)
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
        
        // Create window with default configuration
        WindowConfig windowConfig;
        windowConfig.title = m_appName;
        m_window = std::make_unique<Window>(windowConfig);
        
        // Initialize all systems
        if (!m_systemManager.initialize()) {
            throw ApplicationError("Failed to initialize systems");
        }
        
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
        // Shutdown all systems
        m_systemManager.shutdown();
        
        // Clean up window
        m_window.reset();
        
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

    unsigned int frameCount = 0;
    m_clock.restart();
    m_fpsTimer.restart();

    while (m_gameState != GameState::Stopped && m_window->isOpen()) {
        // Process window events
        if (!m_window->processEvents()) {
            stop();
            continue;
        }

        float deltaTime = m_clock.restart().asSeconds();
        m_accumulatedTime += deltaTime;

        // Update game logic with fixed timestep
        if (m_gameState == GameState::Running) {
            while (m_accumulatedTime >= FIXED_TIMESTEP) {
                update(FIXED_TIMESTEP);
                m_accumulatedTime -= FIXED_TIMESTEP;
            }
        }

        // Begin frame rendering
        m_window->beginFrame();
        
        // Render game state
        render();
        
        // End frame and display
        m_window->endFrame();
        
        frameCount++;

        // Calculate FPS every second
        if (m_fpsTimer.getElapsedTime().asSeconds() >= 1.0f) {
            m_currentFPS = static_cast<float>(frameCount);
            frameCount = 0;
            m_fpsTimer.restart();
        }

        // Frame rate limiting
        if (m_targetFPS > 0) {
            float targetFrameTime = 1.0f / static_cast<float>(m_targetFPS);
            float elapsedTime = m_clock.getElapsedTime().asSeconds();
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
    
    // Update all systems
    m_systemManager.update(deltaTime);
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