#include "Game.h"
#include "graphics/Renderer.h"
#include "graphics/UI.h"
#include "Logger.h"
#include "core/Constants.h"
#include "systems/CameraSystem.h"
#include "systems/LineCreationSystem.h"
#include "systems/StationPlacementSystem.h"

// Constructor no longer takes InputHandler
Game::Game(Renderer& renderer)
    : _renderer(renderer),
      _eventBus(),
      _entityFactory(_registry),
      _worldGenerationSystem(_registry) {
    // Systems will be updated in the next step to take the EventBus
    // For now, this will cause a compile error, which we will fix.
    auto cameraSystem = std::make_unique<CameraSystem>(_eventBus, _camera, _renderer.getWindowInstance());
    auto lineCreationSystem = std::make_unique<LineCreationSystem>(_registry, _entityFactory, _colorManager, _eventBus);
    auto stationPlacementSystem = std::make_unique<StationPlacementSystem>(_registry, _entityFactory, _gameState, _eventBus);

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

// Remove the processInputCommands method entirely

void Game::update(sf::Time dt, UI& ui) {
    // The system manager update now takes only dt
    _systemManager->update(dt);
    
    // The event bus triggers system updates automatically.
    // We just need to trigger the update on the dispatcher.
    _eventBus.update();
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
