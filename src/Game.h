#pragma once

#include <entt/entt.hpp>
#include <memory>
#include "core/Camera.h"
#include "core/EntityFactory.h"
#include "graphics/ColorManager.h"
#include "world/WorldGenerationSystem.h"
#include "core/SystemManager.h" // Include the new manager
#include "systems/CameraSystem.h"
#include "systems/LineCreationSystem.h"
#include "systems/StationPlacementSystem.h"
#include "core/GameState.h"

// Forward declarations
class Renderer;
class InputHandler;
class UI;

class Game {
public:
    Game(Renderer& renderer, InputHandler& inputHandler);
    ~Game();

    void init();
    void update(sf::Time dt, InputHandler& inputHandler, UI& ui);
    void onWindowResize(unsigned int width, unsigned int height);

    entt::registry& getRegistry() { return _registry; }
    Camera& getCamera() { return _camera; }
    WorldGenerationSystem& getWorldGenerationSystem() { return _worldGenerationSystem; }
    size_t getActiveStationCount();
    GameState& getGameState() { return _gameState; }

private:
    void processInputCommands(InputHandler& inputHandler);

    Renderer& _renderer;
    entt::registry _registry;
    EntityFactory _entityFactory;
    ColorManager _colorManager;
    Camera _camera;
    GameState _gameState;
    
    // Systems are now managed by SystemManager
    WorldGenerationSystem _worldGenerationSystem;
    std::unique_ptr<SystemManager> _systemManager;
};
