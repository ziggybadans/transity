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

Game::Game(Renderer &renderer, ThreadPool &threadPool)
    : _renderer(renderer), _eventBus(), _entityFactory(_registry),
      _worldGenerationSystem(_registry, _eventBus) {

    _serviceLocator.registry = &_registry;
    _serviceLocator.eventBus = &_eventBus;
    _serviceLocator.gameState = &_gameState;
    _serviceLocator.entityFactory = &_entityFactory;
    _serviceLocator.camera = &_camera;
    _serviceLocator.colorManager = &_colorManager;
    _serviceLocator.renderer = &_renderer;
    _serviceLocator.threadPool = &threadPool;

    _inputHandler = std::make_unique<InputHandler>(_serviceLocator);

    _systemManager = std::make_unique<SystemManager>(_serviceLocator);

    _systemManager->addSystem<CameraSystem>();
    _systemManager->addSystem<LineCreationSystem>();
    _systemManager->addSystem<StationPlacementSystem>();
    _systemManager->addSystem<GameStateSystem>();
    _systemManager->addSystem<ChunkManagerSystem>(_worldGenerationSystem, _eventBus);

    LOG_INFO("Game", "Game instance created and systems registered.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");

    auto worldGridEntity = _registry.create();
    _registry.emplace<WorldGridComponent>(worldGridEntity);
    LOG_INFO("Game", "WorldGridComponent created with default values.");

    sf::Vector2f worldSize = _worldGenerationSystem.getWorldSize();
    sf::Vector2f worldCenter = {worldSize.x / 2.0f, worldSize.y / 2.0f};

    auto &window = _renderer.getWindowInstance();

    float zoomFactor = 4.0f;
    sf::Vector2f initialViewSize = {worldSize.x / zoomFactor, worldSize.y / zoomFactor};
    _camera.setInitialView(window, worldCenter, initialViewSize);

    sf::Vector2u windowSize = window.getSize();
    _camera.onWindowResize(windowSize.x, windowSize.y);

    LOG_INFO("Game", "Game initialization completed.");
}

void Game::update(sf::Time dt, UI &ui) {

    _systemManager->update(dt);

    _renderer.getTerrainRenderSystem().updateMeshes(_registry);

    _eventBus.update();
}

void Game::onWindowResize(unsigned int width, unsigned int height) {
    _camera.onWindowResize(width, height);
}

size_t Game::getActiveStationCount() {

    auto *lineCreationSystem = _systemManager->getSystem<LineCreationSystem>();
    if (lineCreationSystem) {
        return lineCreationSystem->getActiveLineStations().size();
    }
    return 0;
}

Game::~Game() {
    LOG_INFO("Game", "Game instance destroyed.");
}
