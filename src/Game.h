#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <optional>
#include "Camera.h"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);
    void render();

    sf::RenderWindow window;
    Camera camera;
    sf::Clock deltaClock;
    sf::RectangleShape land;
    sf::Color oceanColor;
};