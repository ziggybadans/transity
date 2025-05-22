#pragma once

#include <SFML/Graphics/RenderWindow.hpp> // Keep for sf::Time if not included elsewhere, or be more specific
#include <SFML/System/Clock.hpp> // For sf::Clock
#include <SFML/System/Time.hpp> // For sf::Time
#include <optional>
#include "Camera.h"
#include <entt/entt.hpp>
#include "InputHandler.h"
#include <memory>
#include "EntityFactory.h"
#include "UI.h"
#include "InteractionMode.h"
#include "LineCreationSystem.h"

class Renderer;

enum GameMode {
    Play
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

    Camera camera;
    sf::Clock deltaClock;

    entt::registry registry;
    EntityFactory m_entityFactory;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputHandler> m_inputHandler;
    std::unique_ptr<UI> m_ui;
    GameMode m_currentGameMode;
    std::unique_ptr<LineCreationSystem> m_lineCreationSystem;
};