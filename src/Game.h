
#pragma once

#include "core/Camera.h"
#include "core/EntityFactory.h"
#include "core/GameState.h"
#include "core/ServiceLocator.h"
#include "core/SystemManager.h"
#include "event/EventBus.h"
#include "graphics/ColorManager.h"
#include "world/WorldGenerationSystem.h"
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
    ServiceLocator& getServiceLocator() { return _serviceLocator; }

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
