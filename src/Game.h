#pragma once

#include <SFML/Graphics/RenderWindow.hpp> // Keep for sf::Time if not included elsewhere, or be more specific
#include <SFML/System/Clock.hpp> // For sf::Clock
#include <SFML/System/Time.hpp> // For sf::Time
#include <optional>
#include "core/Camera.h"
#include <entt/entt.hpp>
#include "input/InputHandler.h"
#include <memory>
#include "core/EntityFactory.h"
#include "graphics/UI.h"
#include "input/InteractionMode.h"
#include "systems/LineCreationSystem.h"

class Renderer;

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
    void processInputCommands(); // New method to handle commands from InputHandler
    void update(sf::Time dt);

    Camera _camera;
    sf::Clock _deltaClock;

    entt::registry _registry;
    EntityFactory _entityFactory;
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<InputHandler> _inputHandler;
    std::unique_ptr<UI> _ui;
    GameMode _currentGameMode;
    std::unique_ptr<LineCreationSystem> _lineCreationSystem;
};