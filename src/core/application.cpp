#include "transity/core/application.hpp"
#include <iostream>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/CircleShape.hpp>

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
    // Register debug commands
    auto& debug = DebugManager::getInstance();
    debug.registerCommand("fps", 
        [this](const std::vector<std::string>& args) {
            if (args.empty()) {
                std::cout << "Current FPS: " << m_currentFPS << std::endl;
            } else {
                setTargetFPS(std::stoul(args[0]));
            }
        },
        "Display or set target FPS (usage: fps [target])"
    );
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
    auto& debug = DebugManager::getInstance();
    debug.beginMetric("initialization", "core", "ms");

    if (m_initialized) {
        throw InitializationError("Application is already initialized");
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
            throw SystemError("Failed to initialize systems");
        }
        
        m_initialized = true;
        debug.log(LogLevel::Info, "Application '" + m_appName + "' initialized successfully");
    }
    catch (const std::exception& e) {
        debug.log(LogLevel::Error, "Initialization failed: " + std::string(e.what()));
        throw InitializationError(std::string("Failed to initialize application: ") + e.what());
    }

    debug.endMetric("initialization");
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }

    auto& debug = DebugManager::getInstance();
    debug.beginMetric("shutdown", "core", "ms");

    try {
        // Shutdown all systems
        m_systemManager.shutdown();
        
        // Clean up window
        m_window.reset();
        
        m_initialized = false;
        debug.log(LogLevel::Info, "Application '" + m_appName + "' shut down successfully");
    }
    catch (const std::exception& e) {
        debug.log(LogLevel::Error, "Error during shutdown: " + std::string(e.what()));
        throw SystemError(std::string("Failed to shutdown application: ") + e.what());
    }

    debug.endMetric("shutdown");
}

void Application::run() {
    auto& debug = DebugManager::getInstance();

    if (!m_initialized) {
        throw StateError("Cannot run application before initialization");
    }

    unsigned int frameCount = 0;
    m_clock.restart();
    m_fpsTimer.restart();

    debug.log(LogLevel::Info, "Starting main loop");

    while (m_gameState != GameState::Stopped && m_window->isOpen()) {
        debug.beginMetric("frame", "core", "ms");

        // Process window events
        if (!m_window->processEvents()) {
            stop();
            continue;
        }

        float deltaTime = m_clock.restart().asSeconds();
        m_accumulatedTime += deltaTime;

        // Update game logic with fixed timestep
        if (m_gameState == GameState::Running) {
            debug.beginMetric("update", "core", "ms");
            while (m_accumulatedTime >= FIXED_TIMESTEP) {
                try {
                    update(FIXED_TIMESTEP);
                } catch (const std::exception& e) {
                    debug.log(LogLevel::Error, "Update error: " + std::string(e.what()));
                    throw RuntimeError("Critical error during update: " + std::string(e.what()));
                }
                m_accumulatedTime -= FIXED_TIMESTEP;
            }
            debug.endMetric("update");
        }

        // Begin frame rendering
        debug.beginMetric("render", "core", "ms");
        m_window->beginFrame();
        
        try {
            // Render game state
            render();
        } catch (const std::exception& e) {
            debug.log(LogLevel::Error, "Render error: " + std::string(e.what()));
            throw RuntimeError("Critical error during render: " + std::string(e.what()));
        }
        
        // End frame and display
        m_window->endFrame();
        debug.endMetric("render");
        
        frameCount++;
        updatePerformanceMetrics();

        // Frame rate limiting
        if (m_targetFPS > 0) {
            float targetFrameTime = 1.0f / static_cast<float>(m_targetFPS);
            float elapsedTime = m_clock.getElapsedTime().asSeconds();
            if (elapsedTime < targetFrameTime) {
                sf::sleep(sf::seconds(targetFrameTime - elapsedTime));
            }
        }

        debug.endMetric("frame");
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
    if (!m_window) return;

    // Draw a simple shape for testing
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(100.f, 100.f);
    m_window->getWindow().draw(shape);
    
    // Clear input states for next frame
    InputManager::getInstance().clear();
}

void Application::pause() {
    if (m_gameState != GameState::Running) {
        throw StateError("Cannot pause: game is not running");
    }
    m_gameState = GameState::Paused;
    DebugManager::getInstance().log(LogLevel::Info, "Game paused");
}

void Application::resume() {
    if (m_gameState != GameState::Paused) {
        throw StateError("Cannot resume: game is not paused");
    }
    m_gameState = GameState::Running;
    DebugManager::getInstance().log(LogLevel::Info, "Game resumed");
}

void Application::stop() {
    m_gameState = GameState::Stopped;
    DebugManager::getInstance().log(LogLevel::Info, "Game stopped");
}

void Application::setTargetFPS(unsigned int fps) {
    if (fps > MAX_FPS) {
        throw StateError("Target FPS exceeds maximum allowed value of " + std::to_string(MAX_FPS));
    }
    m_targetFPS = fps;
    DebugManager::getInstance().log(LogLevel::Info, "Target FPS set to: " + std::to_string(fps));
}

void Application::updatePerformanceMetrics() {
    auto& debug = DebugManager::getInstance();
    
    // Calculate and update FPS
    if (m_fpsTimer.getElapsedTime().asSeconds() >= 1.0f) {
        m_currentFPS = static_cast<float>(m_accumulatedTime);
        debug.addDebugInfo("FPS", std::to_string(static_cast<int>(m_currentFPS)));
        m_fpsTimer.restart();
    }
    
    // Update system metrics
    SystemMetrics metrics;
    metrics.frameTime = m_clock.getElapsedTime().asSeconds() * 1000.0f; // Convert to ms
    metrics.timestamp = std::chrono::steady_clock::now();
    // Note: CPU and memory usage would require platform-specific implementations
    debug.updateSystemMetrics(metrics);
}

} // namespace transity::core 