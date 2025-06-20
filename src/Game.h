// src/Game.h
#pragma once

#include <entt/entt.hpp>
#include <memory>
#include "core/Camera.h"
#include "core/EntityFactory.h"
#include "graphics/ColorManager.h"
#include "world/WorldGenerationSystem.h"
#include "core/SystemManager.h"
#include "core/GameState.h"
#include "event/EventBus.h"
#include "core/ServiceLocator.h"

// Forward declarations
class Renderer;
class UI;
class InputHandler; // Forward-declare InputHandler

class Game {
public:
    Game(Renderer& renderer);
    ~Game();

    void init();
    void update(sf::Time dt, UI& ui);
    void onWindowResize(unsigned int width, unsigned int height);

    entt::registry& getRegistry() { return _registry; }
    EventBus& getEventBus() { return _eventBus; }
    Camera& getCamera() { return _camera; }
    WorldGenerationSystem& getWorldGenerationSystem() { return _worldGenerationSystem; }
    size_t getActiveStationCount();
    GameState& getGameState() { return _gameState; }
    InputHandler& getInputHandler() { return *_inputHandler; } // Add getter

private:
    Renderer& _renderer;
    entt::registry _registry;
    EventBus _eventBus;
    EntityFactory _entityFactory;
    ColorManager _colorManager;
    Camera _camera;
    GameState _gameState;
    
    ServiceLocator _serviceLocator;
    
    WorldGenerationSystem _worldGenerationSystem;
    std::unique_ptr<SystemManager> _systemManager;
    std::unique_ptr<InputHandler> _inputHandler; // Add InputHandler member
};
