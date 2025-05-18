#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <optional>
#include "Camera.h"
#include <entt/entt.hpp>
#include "Renderer.h"
#include <memory>
#include "EntityFactory.h"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);

    sf::RenderWindow window;
    Camera camera;
    sf::Clock deltaClock;

    entt::registry registry;
    EntityFactory m_entityFactory;
    std::unique_ptr<Renderer> m_renderer;
};