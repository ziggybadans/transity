#pragma once

#include "GameState.h"
#include "LoadingState.h"
#include "core/Pathfinder.h"
#include "core/PerformanceMonitor.h"
#include "ecs/EntityFactory.h"
#include "ecs/SystemManager.h"
#include "event/EventBus.h"
#include "render/Camera.h"
#include "render/ColorManager.h"
#include "systems/rendering/PassengerSpawnAnimationSystem.h"
#include "systems/world/WorldGenerationSystem.h"
#include <entt/entt.hpp>
#include <future>
#include <memory>

class Renderer;
class UI;
class InputHandler;
class ChunkManagerSystem;
class ThreadPool;
class Pathfinder;
class CityPlacementSystem;

class Game {
public:
    Game(Renderer &renderer, ThreadPool &threadPool, EventBus &eventBus,
         ColorManager &colorManager);
    ~Game();

    void startLoading();
    void update(sf::Time dt, UI &ui);
    void updateSimulation(sf::Time dt);

    entt::registry &getRegistry() { return _registry; }
    EventBus &getEventBus() { return _eventBus; }
    Camera &getCamera() { return _camera; }
    GameState &getGameState() { return _gameState; }
    LoadingState &getLoadingState() { return _loadingState; }
    InputHandler &getInputHandler() { return *_inputHandler; }
    SystemManager &getSystemManager() { return *_systemManager; }
    WorldGenerationSystem &getWorldGenSystem() { return _worldGenerationSystem; }
    PerformanceMonitor &getPerformanceMonitor() { return _performanceMonitor; }
    CityPlacementSystem &getCityPlacementSystem() {
        return *_simulationSystemManager->getSystem<CityPlacementSystem>();
    }
    PassengerSpawnAnimationSystem &getPassengerSpawnAnimationSystem() {
        return *_systemManager->getSystem<PassengerSpawnAnimationSystem>();
    }

    std::future<void> &getLoadingFuture() { return _loadingFuture; }

private:
    Renderer &_renderer;
    entt::registry _registry;

    EventBus &_eventBus;
    EntityFactory _entityFactory;
    ColorManager &_colorManager;
    Camera _camera;
    GameState _gameState;
    LoadingState _loadingState;
    WorldGenerationSystem _worldGenerationSystem;
    PerformanceMonitor _performanceMonitor;
    Pathfinder _pathfinder;
    ThreadPool &_threadPool;

    std::unique_ptr<SystemManager> _systemManager;
    std::unique_ptr<SystemManager> _simulationSystemManager;
    std::unique_ptr<InputHandler> _inputHandler;

    std::future<void> _loadingFuture;
};