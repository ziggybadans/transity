
#pragma once

#include "Game.h"
#include "core/ThreadPool.h"
#include "render/Renderer.h"
#include "event/EventBus.h"
#include "ui/UI.h"
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
    void renderLoad();

    EventBus _eventBus;
    ColorManager _colorManager;
    std::unique_ptr<ThreadPool> _threadPool;

    std::unique_ptr<Game> _game;
    std::unique_ptr<UI> _ui;
    std::unique_ptr<Renderer> _renderer;

    sf::Clock _deltaClock;
    sf::Time _timeAccumulator;
    const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);

    bool _isWindowFocused = true;
};
