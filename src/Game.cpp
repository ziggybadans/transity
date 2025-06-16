#include "Game.h"
#include "graphics/Renderer.h"
#include "input/InputHandler.h"
#include "Logger.h"
#include "core/Constants.h"
#include <cstdlib>
#include <memory>
#include <string>

Game::Game()
    : _entityFactory(_registry),
    _worldGenerationSystem(_registry),
    _cameraSystem(std::make_unique<CameraSystem>()),
    _stationPlacementSystem(std::make_unique<StationPlacementSystem>()) {
    _lineCreationSystem = std::make_unique<LineCreationSystem>(_registry, _entityFactory, _colorManager);
    
    LOG_INFO("Game", "Game instance creating.");
    LOG_INFO("Game", "Game instance created successfully.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");
    LOG_INFO("Main", "Logger initialized. Enabled: %s, MinLevel: %s", Logging::Logger::getInstance().isLoggingEnabled() ? "true" : "false", Logging::Logger::getInstance().logLevelToString(Logging::Logger::getInstance().getMinLogLevel()));
    LOG_INFO("Main", "Default global log delay set to: %ums", Logging::Logger::getInstance().getLogDelay());
    LOG_INFO("Main", "Application starting.");
    try {
        _renderer = std::make_unique<Renderer>();
        if (!_renderer) {
            LOG_FATAL("Game", "Failed to create Renderer instance (_renderer is null).");
            exit(EXIT_FAILURE);
        }
        _renderer->initialize();

        sf::Vector2f worldSize = _worldGenerationSystem.getWorldSize();
        sf::Vector2f worldCenter = { worldSize.x / 2.0f, worldSize.y / 2.0f };
        _camera.setInitialView(_renderer->getWindowInstance(), worldCenter, worldSize);

        sf::Vector2u windowSize = _renderer->getWindowInstance().getSize();
        _camera.onWindowResize(windowSize.x, windowSize.y);

        _ui = std::make_unique<UI>(_renderer->getWindowInstance(), &_worldGenerationSystem);
        if (!_ui) {
            LOG_FATAL("Game", "Failed to create UI instance (_ui is null).");
            exit(EXIT_FAILURE);
        }
        _ui->initialize();

        _inputHandler = std::make_unique<InputHandler>();
        if (!_inputHandler) {
            LOG_FATAL("Game", "Failed to create InputHandler instance (_inputHandler is null).");
            exit(EXIT_FAILURE);
        }
    } catch (const std::bad_alloc& e) {
        LOG_FATAL("Game", "Failed to allocate memory for core systems: %s", e.what());
        exit(EXIT_FAILURE);
    } catch (const std::exception& e) {
        LOG_FATAL("Game", "An unexpected exception occurred during core system creation: %s", e.what());
        exit(EXIT_FAILURE);
    } catch (...) {
        LOG_FATAL("Game", "An unknown exception occurred during core system creation.");
        exit(EXIT_FAILURE);
    }

    Logging::Logger::getInstance().setLogLevelDelay(Logging::LogLevel::TRACE, Constants::TRACE_LOG_DELAY_MS);
    LOG_INFO("Main", "TRACE log delay set to: %ums", Logging::Logger::getInstance().getLogLevelDelay(Logging::LogLevel::TRACE));
    LOG_INFO("Game", "Game initialization completed.");
}

void Game::processInputCommands() {
    const auto& commands = _inputHandler->getCommands();
    for (const auto& command : commands) {
        switch (command.type) {
            case InputEventType::WINDOW_CLOSE:
                LOG_INFO("Game", "Processing WindowClose command.");
                _renderer->getWindowInstance().close();
                break;
            case InputEventType::NONE:
            default:
                break;
        }
    }
    
    _inputHandler->clearCommands();
}


void Game::run() {
    LOG_INFO("Game", "Starting game loop.");
    while (_renderer->getWindowInstance().isOpen()) {
        sf::Time dt = _deltaClock.restart();
        LOG_TRACE("Game", "Delta time: %f seconds", dt.asSeconds());

        while (auto optEvent = _renderer->getWindowInstance().pollEvent()) {
            if (optEvent) {
                const sf::Event& currentEvent = *optEvent;

                if (currentEvent.is<sf::Event::FocusLost>()) {
                    LOG_DEBUG("Game", "Window focus lost.");
                    _isWindowFocused = false;
                } else if (currentEvent.is<sf::Event::FocusGained>()) {
                    LOG_DEBUG("Game", "Window focus gained.");
                    _isWindowFocused = true;
                }

                if (const auto* resizedEvent = currentEvent.getIf<sf::Event::Resized>()) {
                    _camera.onWindowResize(resizedEvent->size.x, resizedEvent->size.y);
                }

                _ui->processEvent(currentEvent);
                if (_isWindowFocused) {
                    _inputHandler->handleGameEvent(currentEvent, _ui->getInteractionMode(), _camera, _renderer->getWindowInstance(), _registry);
                }
            }
        }
        
        if (_isWindowFocused) {
            _inputHandler->update(dt, _camera);
            _cameraSystem->update(*_inputHandler, _camera, _renderer->getWindowInstance());
            _stationPlacementSystem->update(*_inputHandler, *_ui, _registry, _entityFactory);
        }
        processInputCommands();

        _ui->update(dt, _lineCreationSystem->getActiveLineStations().size());

        _lineCreationSystem->processEvents(_inputHandler->getGameEvents(), _ui->getUiEvents());

        _inputHandler->clearGameEvents();
        _ui->clearUiEvents();

        update(dt);
        LOG_TRACE("Game", "Game logic updated.");

        _renderer->renderFrame(_registry, _camera.getView(), dt, _ui->getVisualizeNoiseState());
        LOG_TRACE("Game", "Frame rendered.");

        _ui->renderFrame();

        _renderer->displayFrame();
    }
    LOG_INFO("Game", "Game loop ended.");
    _renderer->cleanupResources();
    _ui->cleanupResources();
    LOG_INFO("Game", "Renderer and UI cleaned up.");
}

void Game::update(sf::Time dt) {
}

Game::~Game() {
    LOG_INFO("Main", "Application shutting down.");
}