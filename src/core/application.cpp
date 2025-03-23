#include "transity/core/application.hpp"
#include <iostream>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <mutex>

namespace transity::core {

Application::Application()
    : m_initialized(false)
    , m_appName("")
    , m_gameState(GameState::Stopped)
    , m_targetFPS(60)
    , m_currentFPS(0.0f)
    , m_accumulatedTime(0.0f)
    , m_mainWindowId("main")
    , m_isUpdating(false)
    , m_isRendering(false)
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
        
        // Create main window with default configuration
        WindowConfig windowConfig;
        windowConfig.title = m_appName;
        createWindow(m_mainWindowId, windowConfig);
        
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
        
        // Clean up all windows
        WindowManager::getInstance().cleanup();
        
        m_initialized = false;
        debug.log(LogLevel::Info, "Application '" + m_appName + "' shut down successfully");
    }
    catch (const std::exception& e) {
        debug.log(LogLevel::Error, "Error during shutdown: " + std::string(e.what()));
        throw SystemError(std::string("Failed to shutdown application: ") + e.what());
    }

    debug.endMetric("shutdown");
}

Window& Application::createWindow(const std::string& windowId, const WindowConfig& config) {
    return WindowManager::getInstance().createWindow(windowId, config);
}

Window& Application::getWindow(const std::string& windowId) {
    return WindowManager::getInstance().getWindow(windowId);
}

Window& Application::getMainWindow() {
    return getWindow(m_mainWindowId);
}

void Application::setGameState(GameState newState) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    // Validate state transitions
    switch (m_gameState) {
        case GameState::Running:
            if (newState != GameState::Paused && newState != GameState::Stopped && newState != GameState::Error) {
                throw StateError("Invalid state transition from Running");
            }
            break;
        case GameState::Paused:
            if (newState != GameState::Running && newState != GameState::Stopped && newState != GameState::Error) {
                throw StateError("Invalid state transition from Paused");
            }
            break;
        case GameState::Stopped:
            if (newState != GameState::Running && newState != GameState::Error) {
                throw StateError("Invalid state transition from Stopped");
            }
            break;
        case GameState::Error:
            if (newState != GameState::Running && newState != GameState::Stopped) {
                throw StateError("Invalid state transition from Error");
            }
            break;
    }
    
    m_gameState = newState;
    std::string stateStr;
    switch (newState) {
        case GameState::Running: stateStr = "Running"; break;
        case GameState::Paused: stateStr = "Paused"; break;
        case GameState::Stopped: stateStr = "Stopped"; break;
        case GameState::Error: stateStr = "Error"; break;
    }
    DebugManager::getInstance().log(LogLevel::Info, "Game state changed to: " + stateStr);
}

Application::GameState Application::getGameState() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_gameState;
}

void Application::run() {
    if (!m_initialized) {
        throw StateError("Application not initialized");
    }

    setGameState(GameState::Running);
    m_accumulatedTime = 0.0f;

    try {
        while (getGameState() != GameState::Stopped) {
            if (getGameState() == GameState::Error) {
                if (!attemptRecovery()) {
                    break;
                }
                setGameState(GameState::Running);
            }

            float deltaTime = m_clock.restart().asSeconds();
            m_accumulatedTime += deltaTime;

            processInput();

            // Fixed timestep updates with thread safety
            while (m_accumulatedTime >= FIXED_TIMESTEP) {
                std::unique_lock<std::mutex> updateLock(m_updateMutex);
                m_isUpdating = true;
                
                try {
                    update(FIXED_TIMESTEP);
                } catch (const RecoverableError& e) {
                    m_isUpdating = false;
                    updateLock.unlock();
                    if (!handleError(e)) {
                        throw;
                    }
                    continue;
                }
                
                m_isUpdating = false;
                updateLock.unlock();
                m_accumulatedTime -= FIXED_TIMESTEP;
            }

            // Thread-safe rendering
            std::unique_lock<std::mutex> renderLock(m_renderMutex);
            m_isRendering = true;
            
            try {
                render();
            } catch (const RecoverableError& e) {
                m_isRendering = false;
                renderLock.unlock();
                if (!handleError(e)) {
                    throw;
                }
            }
            
            m_isRendering = false;
            renderLock.unlock();

            updatePerformanceMetrics();

            // Check if all windows are closed
            if (!WindowManager::getInstance().hasOpenWindows()) {
                stop();
            }
        }
    } catch (const TransityError& e) {
        handleError(e);
        throw;
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
        if (getGameState() == GameState::Running) {
            pause();
        } else if (getGameState() == GameState::Paused) {
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
    auto& windowManager = WindowManager::getInstance();
    windowManager.beginFrame();

    // Draw to all windows
    for (size_t i = 0; i < windowManager.getWindowCount(); ++i) {
        // Draw a simple shape for testing
        sf::CircleShape shape(50.f);
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(100.f, 100.f);
        getMainWindow().getWindow().draw(shape);
    }

    windowManager.endFrame();
    
    // Clear input states for next frame
    InputManager::getInstance().clear();
}

void Application::pause() {
    if (getGameState() != GameState::Running) {
        throw StateError("Cannot pause: game is not running");
    }
    setGameState(GameState::Paused);
}

void Application::resume() {
    if (getGameState() != GameState::Paused) {
        throw StateError("Cannot resume: game is not paused");
    }
    setGameState(GameState::Running);
}

void Application::stop() {
    setGameState(GameState::Stopped);
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

void Application::registerErrorHandler(ErrorCallback callback) {
    m_errorHandlers.push_back(std::move(callback));
}

void Application::registerRecoveryHandler(RecoveryCallback callback) {
    m_recoveryHandlers.push_back(std::move(callback));
}

void Application::clearError() {
    m_lastError.reset();
    if (getGameState() == GameState::Error) {
        setGameState(GameState::Stopped);
    }
}

bool Application::attemptRecovery() {
    if (!hasError()) {
        return true;
    }

    auto* recoverableError = dynamic_cast<const RecoverableError*>(m_lastError.get());
    if (!recoverableError) {
        return false;
    }

    bool recovered = false;
    for (const auto& handler : m_recoveryHandlers) {
        if (handler(*recoverableError)) {
            recovered = true;
            break;
        }
    }

    if (recovered) {
        clearError();
        return true;
    }

    return false;
}

bool Application::handleError(const TransityError& error) {
    // Store the error
    setError(std::make_unique<TransityError>(error));

    // Notify all error handlers
    for (const auto& handler : m_errorHandlers) {
        handler(error);
    }

    // Check if this is a recoverable error
    if (auto* recoverableError = dynamic_cast<const RecoverableError*>(&error)) {
        return attemptRecovery();
    }

    // For non-recoverable errors, transition to error state
    setGameState(GameState::Error);
    return false;
}

void Application::setError(std::unique_ptr<TransityError> error) {
    m_lastError = std::move(error);
}

} // namespace transity::core 