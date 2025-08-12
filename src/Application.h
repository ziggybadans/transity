
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
    void render(float interpolation);

    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<UI> _ui;
    std::unique_ptr<Game> _game;

    sf::Clock _deltaClock;
    sf::Time _timeAccumulator;
    const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);

    bool _isWindowFocused = true;
};
