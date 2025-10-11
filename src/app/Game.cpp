#include "Game.h"
#include "Constants.h"
#include "Logger.h"
#include "core/ThreadPool.h"
#include "input/InputHandler.h"
#include "render/Renderer.h"
#include "systems/app/GameStateSystem.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "systems/gameplay/DeletionSystem.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "systems/gameplay/LineEditingSystem.h"
#include "systems/gameplay/PassengerMovementSystem.h"
#include "systems/gameplay/PassengerSpawnSystem.h"
#include "systems/gameplay/SelectionSystem.h"
#include "systems/gameplay/StationPlacementSystem.h"
#include "systems/gameplay/TrainMovementSystem.h"
#include "systems/rendering/CameraSystem.h"
#include "systems/rendering/PassengerSpawnAnimationSystem.h"
#include "systems/rendering/TerrainMeshSystem.h"
#include "systems/world/ChunkManagerSystem.h"
#include "systems/world/WorldSetupSystem.h"
#include "systems/gameplay/SharedSegmentSystem.h"
#include "systems/gameplay/ScoreSystem.h"
#include "systems/gameplay/LineDataSystem.h"
#include "ui/UI.h"

Game::Game(Renderer &renderer, ThreadPool &threadPool, EventBus &eventBus,
           ColorManager &colorManager)
    : _renderer(renderer), _eventBus(eventBus), _colorManager(colorManager),
      _entityFactory(_registry, "data/archetypes"), _worldGenerationSystem(_registry, _eventBus),
      _pathfinder(_registry), _threadPool(threadPool) {

    _inputHandler = std::make_unique<InputHandler>(_eventBus, _camera);
    _systemManager = std::make_unique<SystemManager>();
    _simulationSystemManager = std::make_unique<SystemManager>();

    // UI, input, and world loading systems that should always run
    _systemManager->addSystem<CameraSystem>(_camera, _renderer, _worldGenerationSystem, _eventBus);
    _systemManager->addSystem<LineCreationSystem>(_registry, _entityFactory, _colorManager,
                                                  _gameState, _eventBus, _worldGenerationSystem);
    _systemManager->addSystem<GameStateSystem>(_eventBus, _gameState);
    _systemManager->addSystem<SelectionSystem>(_registry, _eventBus, _gameState, _pathfinder);
    _systemManager->addSystem<DeletionSystem>(_registry, _eventBus, _gameState);
    _systemManager->addSystem<LineEditingSystem>(_registry, _eventBus, _gameState);
    _systemManager->addSystem<SharedSegmentSystem>(_registry, _eventBus);
    _systemManager->addSystem<ChunkManagerSystem>(_registry, _eventBus, _worldGenerationSystem,
                                                  _camera, _threadPool);
    _systemManager->addSystem<TerrainMeshSystem>(_registry, _renderer, _worldGenerationSystem,
                                                 _eventBus);
    _systemManager->addSystem<PassengerSpawnAnimationSystem>(_registry, _entityFactory,
                                                             _pathfinder);

    // Simulation systems that should be paused
    _simulationSystemManager->addSystem<WorldSetupSystem>(
        _registry, _loadingState, _worldGenerationSystem, _renderer, _camera);
    _simulationSystemManager->addSystem<CityPlacementSystem>(_loadingState, _worldGenerationSystem,
                                                             _entityFactory, _renderer,
                                                             _performanceMonitor, _threadPool);
    _simulationSystemManager->addSystem<TrainMovementSystem>(_registry);
    _simulationSystemManager->addSystem<PassengerMovementSystem>(_registry);
    _simulationSystemManager->addSystem<PassengerSpawnSystem>(_registry, _entityFactory,
                                                              _pathfinder);
    _simulationSystemManager->addSystem<ScoreSystem>(_registry);
    _simulationSystemManager->addSystem<LineDataSystem>(_registry, _entityFactory, _eventBus);

    LOG_INFO("Game", "Game instance created and systems registered.");
}

void Game::update(sf::Time dt, UI &ui) {
    _systemManager->update(dt);
}

void Game::updateSimulation(sf::Time dt) {
    _simulationSystemManager->update(dt);
}

void Game::startLoading() {
    _loadingState.progress = 0.0f;
    _loadingState.message = "Loading...";

    auto worldSetupSystem = _simulationSystemManager->getSystem<WorldSetupSystem>();
    auto cityPlacementSystem = _simulationSystemManager->getSystem<CityPlacementSystem>();

    _loadingFuture = _threadPool.enqueue([worldSetupSystem, cityPlacementSystem]() {
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