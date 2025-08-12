#include "Game.h"
#include "Logger.h"
#include "core/Constants.h"
#include "graphics/Renderer.h"
#include "graphics/UI.h"
#include "input/InputHandler.h"
#include "systems/CameraSystem.h"
#include "systems/GameStateSystem.h"
#include "systems/LineCreationSystem.h"
#include "systems/StationPlacementSystem.h"
#include "world/ChunkManagerSystem.h"
#include "core/ThreadPool.h"
#include "systems/WorldSetupSystem.h"

Game::Game(Renderer &renderer, ThreadPool &threadPool)
    : _renderer(renderer), _eventBus(), _entityFactory(_registry),
      _worldGenerationSystem(_registry, _eventBus) {

    _serviceLocator.registry = &_registry;
    _serviceLocator.eventBus = &_eventBus;
    _serviceLocator.gameState = &_gameState;
    _serviceLocator.entityFactory = &_entityFactory;
    _serviceLocator.camera = &_camera;
    _serviceLocator.colorManager = &_colorManager;
    _serviceLocator.worldGenerationSystem = &_worldGenerationSystem;
    _serviceLocator.renderer = &_renderer;
    _serviceLocator.threadPool = &threadPool;

    _inputHandler = std::make_unique<InputHandler>(_serviceLocator);

    _systemManager = std::make_unique<SystemManager>(_serviceLocator);

    _systemManager->addSystem<CameraSystem>();
    _systemManager->addSystem<LineCreationSystem>();
    _systemManager->addSystem<StationPlacementSystem>();
    _systemManager->addSystem<GameStateSystem>();
    _systemManager->addSystem<WorldSetupSystem>()->init();
    _systemManager->addSystem<ChunkManagerSystem>(_worldGenerationSystem, _eventBus);

    LOG_INFO("Game", "Game instance created and systems registered.");
}

void Game::update(sf::Time dt, UI &ui) {

    _systemManager->update(dt);

    _renderer.getTerrainRenderSystem().updateMeshes(_registry);

    _eventBus.update();
}

Game::~Game() {
    LOG_INFO("Game", "Game instance destroyed.");
}
