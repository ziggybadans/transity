#include "Game.h"
#include "graphics/Renderer.h"
#include "input/InputHandler.h"
#include "graphics/UI.h"
#include "Logger.h"
#include "core/Constants.h"

Game::Game(Renderer& renderer, InputHandler& inputHandler)
    : _renderer(renderer),
      _entityFactory(_registry),
      _worldGenerationSystem(_registry) {
    // 1. Create instances of the systems, injecting their dependencies.
    auto cameraSystem = std::make_unique<CameraSystem>(inputHandler, _camera, _renderer.getWindowInstance());
    auto lineCreationSystem = std::make_unique<LineCreationSystem>(_registry, _entityFactory, _colorManager);
    auto stationPlacementSystem = std::make_unique<StationPlacementSystem>(inputHandler, _registry, _entityFactory);

    // 2. Create the SystemManager, passing ownership of the systems.
    _systemManager = std::make_unique<SystemManager>(
        std::move(cameraSystem),
        std::move(lineCreationSystem),
        std::move(stationPlacementSystem)
    );

    LOG_INFO("Game", "Game instance created.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");
    
    sf::Vector2f worldSize = _worldGenerationSystem.getWorldSize();
    sf::Vector2f worldCenter = { worldSize.x / 2.0f, worldSize.y / 2.0f };
    
    auto& window = _renderer.getWindowInstance();
    _camera.setInitialView(window, worldCenter, worldSize);

    sf::Vector2u windowSize = window.getSize();
    _camera.onWindowResize(windowSize.x, windowSize.y);

    LOG_INFO("Game", "Game initialization completed.");
}

void Game::processInputCommands(InputHandler& inputHandler) {
    const auto& commands = inputHandler.getCommands();
    for (const auto& command : commands) {
        // Game-specific command processing
    }
    inputHandler.clearCommands();
}

void Game::update(sf::Time dt, InputHandler& inputHandler, UI& ui) {
    _systemManager->update(dt, _gameState.currentInteractionMode);

    _systemManager->processEvents(inputHandler, ui);

    processInputCommands(inputHandler);

    inputHandler.clearGameEvents();
    ui.clearUiEvents();
}

void Game::onWindowResize(unsigned int width, unsigned int height) {
    _camera.onWindowResize(width, height);
}

size_t Game::getActiveStationCount() {
    return _systemManager->getLineCreationSystem().getActiveLineStations().size();
}

Game::~Game() {
    LOG_INFO("Game", "Game instance destroyed.");
}
