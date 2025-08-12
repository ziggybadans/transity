
#pragma once

#include "Game.h"
#include "graphics/Renderer.h"
#include "graphics/UI.h"
#include <SFML/System/Clock.hpp>
#include <memory>

class Application {
public:
    Application();
    void run();

private:
    void processEvents();
    void update(sf::Time dt);
    void render(sf::Time dt);

    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<UI> _ui;
    std::unique_ptr<Game> _game;

    sf::Clock _deltaClock;
    bool _isWindowFocused = true;
};
