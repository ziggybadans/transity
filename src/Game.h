#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <optional>
#include "core/Camera.h"
#include <entt/entt.hpp>
#include "input/InputHandler.h"
#include <memory>
#include "core/EntityFactory.h"
#include "graphics/UI.h"
#include "input/InteractionMode.h"
#include "systems/LineCreationSystem.h"
#include "world/WorldGenerationSystem.h"
#include "systems/CameraSystem.h"
#include "systems/StationPlacementSystem.h"

class Renderer;
#include "graphics/ColorManager.h"

enum GameMode {
    PLAY
};

class Game {
public:
    Game();
    ~Game();
    void init();
    void run();

private:
    void processInputCommands();
    void update(sf::Time dt);

    Camera _camera;
    sf::Clock _deltaClock;

    entt::registry _registry;
    EntityFactory _entityFactory;
    ColorManager _colorManager;
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<InputHandler> _inputHandler;
    std::unique_ptr<UI> _ui;
    GameMode _currentGameMode;
    std::unique_ptr<LineCreationSystem> _lineCreationSystem;
    WorldGenerationSystem _worldGenerationSystem;
    std::unique_ptr<CameraSystem> _cameraSystem;
    std::unique_ptr<StationPlacementSystem> _stationPlacementSystem;

    bool _isWindowFocused = true;
};