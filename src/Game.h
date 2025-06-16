#pragma once

#include <entt/entt.hpp>
#include <memory>
#include "core/Camera.h"
#include "core/EntityFactory.h"
#include "graphics/ColorManager.h"
#include "systems/LineCreationSystem.h"
#include "world/WorldGenerationSystem.h"
#include "systems/CameraSystem.h"
#include "systems/StationPlacementSystem.h"
#include <SFML/System/Time.hpp>

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

private:
    void processInputCommands(InputHandler& inputHandler);

    Renderer& _renderer; // Store a reference to the renderer
    entt::registry _registry;
    EntityFactory _entityFactory;
    ColorManager _colorManager;
    Camera _camera;
    
    // Systems
    WorldGenerationSystem _worldGenerationSystem;
    std::unique_ptr<LineCreationSystem> _lineCreationSystem;
    std::unique_ptr<CameraSystem> _cameraSystem;
    std::unique_ptr<StationPlacementSystem> _stationPlacementSystem;
};
