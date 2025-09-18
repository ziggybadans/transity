#include "Game.h"
#include "Constants.h"
#include "Logger.h"
#include "core/ThreadPool.h"
#include "input/InputHandler.h"
#include "render/Renderer.h"
#include "systems/app/GameStateSystem.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "systems/gameplay/StationPlacementSystem.h"
#include "systems/rendering/CameraSystem.h"
#include "systems/rendering/TerrainMeshSystem.h"
#include "systems/world/ChunkManagerSystem.h"
#include "systems/world/WorldSetupSystem.h"
#include "systems/gameplay/TrainMovementSystem.h"
#include "systems/gameplay/SelectionSystem.h"
#include "ui/UI.h"

Game::Game(Renderer &renderer, ThreadPool &threadPool, EventBus &eventBus, ColorManager &colorManager)
    : _renderer(renderer), _eventBus(eventBus), _colorManager(colorManager), _entityFactory(_registry),
      _worldGenerationSystem(_registry, _eventBus),
      _serviceLocator{_registry,      _eventBus,  _loadingState,      _gameState,
                      _entityFactory, _camera,    _colorManager,      _worldGenerationSystem,
                      _renderer,      threadPool, _performanceMonitor} {

    _inputHandler = std::make_unique<InputHandler>(_serviceLocator);
    _systemManager = std::make_unique<SystemManager>(_serviceLocator);

    _systemManager->addSystem<CameraSystem>();
    _systemManager->addSystem<LineCreationSystem>();
    _systemManager->addSystem<GameStateSystem>();
    _systemManager->addSystem<WorldSetupSystem>();
    _systemManager->addSystem<ChunkManagerSystem>(_worldGenerationSystem, _eventBus);
    _systemManager->addSystem<TerrainMeshSystem>();
    _systemManager->addSystem<CityPlacementSystem>();
    _systemManager->addSystem<SelectionSystem>();
    _systemManager->addSystem<TrainMovementSystem>();

    LOG_INFO("Game", "Game instance created and systems registered.");
}

void Game::update(sf::Time dt, UI &ui) {
    _systemManager->update(dt);
}

void Game::startLoading() {
    _loadingState.progress = 0.0f;
    _loadingState.message = "Loading...";

    auto &threadPool = _serviceLocator.threadPool;
    auto worldSetupSystem = _systemManager->getSystem<WorldSetupSystem>();
    auto cityPlacementSystem = _systemManager->getSystem<CityPlacementSystem>();

    _loadingFuture = threadPool.enqueue([worldSetupSystem, cityPlacementSystem]() {
        if (worldSetupSystem) {
            worldSetupSystem->init();
        }
        if (cityPlacementSystem) {
            cityPlacementSystem->init();
        }
    });
}

Game::~Game() {
    LOG_INFO("Game", "Game instance destroyed.");
}