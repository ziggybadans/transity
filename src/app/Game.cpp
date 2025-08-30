#include "Game.h"
#include "Constants.h"
#include "Logger.h"
#include "core/ThreadPool.h"
#include "input/InputHandler.h"
#include "render/Renderer.h"
#include "systems/app/GameStateSystem.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "systems/gameplay/StationPlacementSystem.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include "systems/rendering/CameraSystem.h"
#include "systems/rendering/TerrainMeshSystem.h"
#include "systems/world/ChunkManagerSystem.h"
#include "systems/world/WorldSetupSystem.h"
#include "ui/UI.h"

Game::Game(Renderer &renderer, ThreadPool &threadPool)
    : _renderer(renderer), _eventBus(), _entityFactory(_registry),
      _worldGenerationSystem(_registry, _eventBus), _serviceLocator{_registry,
                                                                    _eventBus,
                                                                    _gameState,
                                                                    _entityFactory,
                                                                    _camera,
                                                                    _colorManager,
                                                                    _worldGenerationSystem,
                                                                    _renderer,
                                                                    threadPool} {

    _inputHandler = std::make_unique<InputHandler>(_serviceLocator);
    _systemManager = std::make_unique<SystemManager>(_serviceLocator);

    _systemManager->addSystem<CameraSystem>();
    _systemManager->addSystem<LineCreationSystem>();
    _systemManager->addSystem<GameStateSystem>();
    _systemManager->addSystem<WorldSetupSystem>()->init();
    _systemManager->addSystem<ChunkManagerSystem>(_worldGenerationSystem, _eventBus);
    _systemManager->addSystem<TerrainMeshSystem>();
    _systemManager->addSystem<CityPlacementSystem>();

    LOG_INFO("Game", "Game instance created and systems registered.");
}

void Game::update(sf::Time dt, UI &ui) {

    _systemManager->update(dt);

    _eventBus.update();
}

Game::~Game() {
    LOG_INFO("Game", "Game instance destroyed.");
}