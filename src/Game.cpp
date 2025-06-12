#include "Game.h"
#include "graphics/Renderer.h"
#include "input/InputHandler.h"
#include "Logger.h"
#include <cstdlib>
#include <memory>
#include <string>

Game::Game()
    : _entityFactory(_registry),
    _worldGenerationSystem(_registry) {
    _lineCreationSystem = std::make_unique<LineCreationSystem>(_registry, _entityFactory);
    
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

    Logging::Logger::getInstance().setLogLevelDelay(Logging::LogLevel::TRACE, 2000);
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
            case InputEventType::CAMERA_ZOOM:
                {
                    LOG_DEBUG("Game", "Processing CameraZoom command with delta: %.2f", command.data.zoomDelta);
                    sf::View& view = _camera.getViewToModify();
                    sf::Vector2f worldPosBeforeZoom = _renderer->getWindowInstance().mapPixelToCoords(command.data.mousePixelPosition, view);
                    _camera.zoomView(command.data.zoomDelta);
                    sf::Vector2f worldPosAfterZoom = _renderer->getWindowInstance().mapPixelToCoords(command.data.mousePixelPosition, view);
                    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
                    _camera.moveView(offset);
                    LOG_TRACE("Game", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x, offset.y);
                }
                break;
            case InputEventType::CAMERA_PAN:
                LOG_DEBUG("Game", "Processing CameraPan command with direction: (%.1f, %.1f)", command.data.panDirection.x, command.data.panDirection.y);
                _camera.moveView(command.data.panDirection);
                break;
            case InputEventType::TRY_PLACE_STATION:
                if (_ui->getInteractionMode() == InteractionMode::CREATE_STATION) {
                    LOG_DEBUG("Game", "Processing TryPlaceStation command at (%.1f, %.1f)", command.data.worldPosition.x, command.data.worldPosition.y);
                    int nextStationId = _registry.alive() ? static_cast<int>(_registry.size()) : 0;
                    _entityFactory.createStation(command.data.worldPosition, "New Station " + std::to_string(nextStationId));
                }
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
                _ui->processEvent(currentEvent);
                _inputHandler->handleGameEvent(currentEvent, _ui->getInteractionMode(), _camera, _renderer->getWindowInstance(), _registry);
            }
        }
        
        _inputHandler->update(dt);
        processInputCommands();

        _ui->update(dt, _lineCreationSystem->getActiveLineStations().size());

        _lineCreationSystem->processEvents(_inputHandler->getGameEvents(), _ui->getUiEvents());

        _inputHandler->clearGameEvents();
        _ui->clearUiEvents();

        update(dt);
        LOG_TRACE("Game", "Game logic updated.");

        _renderer->renderFrame(_registry, _camera.getView(), dt, _ui->getInteractionMode(), _ui->getVisualizeNoiseState());
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