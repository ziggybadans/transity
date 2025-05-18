#pragma once

#include <SFML/Graphics/RenderWindow.hpp> // Keep for sf::Time if not included elsewhere, or be more specific
#include <SFML/System/Clock.hpp> // For sf::Clock
#include <SFML/System/Time.hpp> // For sf::Time
#include <optional>
#include "Camera.h"
#include <entt/entt.hpp>
#include "Renderer.h"
#include "InputHandler.h" // Added
#include <memory>
#include "EntityFactory.h"


enum class GameMode {
    StationPlacement,
    LineCreation,
    None
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
    
    GameMode m_currentGameMode = GameMode::None;
};