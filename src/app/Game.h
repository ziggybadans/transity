#pragma once

#include "GameState.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "ecs/SystemManager.h"
#include "event/EventBus.h"
#include "render/Camera.h"
#include "render/ColorManager.h"
#include "systems/world/WorldGenerationSystem.h"
#include "systems/gameplay/CityPlacementSystem.h"
#include <entt/entt.hpp>
#include <memory>

class Renderer;
class UI;
class InputHandler;
class ChunkManagerSystem;
class ThreadPool;

class Game {
public:
    Game(Renderer &renderer, ThreadPool &threadPool);
    ~Game();

    void update(sf::Time dt, UI &ui);

    entt::registry &getRegistry() { return _registry; }
    EventBus &getEventBus() { return _eventBus; }
    Camera &getCamera() { return _camera; }
    GameState &getGameState() { return _gameState; }
    InputHandler &getInputHandler() { return *_inputHandler; }
    ServiceLocator &getServiceLocator() { return _serviceLocator; }
    const WorldGenerationSystem &getWorldGenSystem() const { return _worldGenerationSystem; }

private:
    Renderer &_renderer;
    entt::registry _registry;

    EventBus _eventBus;
    EntityFactory _entityFactory;
    ColorManager _colorManager;
    Camera _camera;
    GameState _gameState;
    WorldGenerationSystem _worldGenerationSystem;
    ServiceLocator _serviceLocator;

    std::unique_ptr<ChunkManagerSystem> _chunkManagerSystem;
    std::unique_ptr<SystemManager> _systemManager;
    std::unique_ptr<InputHandler> _inputHandler;
};