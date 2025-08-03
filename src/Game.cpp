#include "Game.h"
#include "graphics/Renderer.h"
#include "graphics/UI.h"
#include "Logger.h"
#include "core/Constants.h"
#include "systems/CameraSystem.h"
#include "systems/LineCreationSystem.h"
#include "systems/StationPlacementSystem.h"
#include "input/InputHandler.h"
#include "systems/GameStateSystem.h"
#include "world/ChunkManagerSystem.h"

// Constructor no longer takes InputHandler
Game::Game(Renderer& renderer)
    : _renderer(renderer),
      _eventBus(),
      _entityFactory(_registry),
      _worldGenerationSystem(_registry, _eventBus) {

    // 1. Populate the ServiceLocator with all the core services.
    _serviceLocator.registry = &_registry;
    _serviceLocator.eventBus = &_eventBus;
    _serviceLocator.gameState = &_gameState;
    _serviceLocator.entityFactory = &_entityFactory;
    _serviceLocator.camera = &_camera;
    _serviceLocator.colorManager = &_colorManager;
    _serviceLocator.renderer = &_renderer;

    // 2. Create the InputHandler using the ServiceLocator.
    _inputHandler = std::make_unique<InputHandler>(_serviceLocator);

    // 3. Create the SystemManager, passing it the ServiceLocator.
    _systemManager = std::make_unique<SystemManager>(_serviceLocator);

    _chunkManagerSystem = std::make_unique<ChunkManagerSystem>(_serviceLocator, _worldGenerationSystem);

    // 4. Add systems using the new templated method.
    _systemManager->addSystem<CameraSystem>();
    _systemManager->addSystem<LineCreationSystem>();
    _systemManager->addSystem<StationPlacementSystem>();
    _systemManager->addSystem<GameStateSystem>();
    _systemManager->addSystem<ChunkManagerSystem>(_worldGenerationSystem);

    LOG_INFO("Game", "Game instance created and systems registered.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");

    // Create and emplace the WorldGridComponent singleton
    auto worldGridEntity = _registry.create();
    _registry.emplace<WorldGridComponent>(worldGridEntity);
    LOG_INFO("Game", "WorldGridComponent created with default values.");
    
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
    // Retrieve the system from the manager to get the data.
    auto* lineCreationSystem = _systemManager->getSystem<LineCreationSystem>();
    if (lineCreationSystem) {
        return lineCreationSystem->getActiveLineStations().size();
    }
    return 0;
}


Game::~Game() {
    LOG_INFO("Game", "Game instance destroyed.");
}
