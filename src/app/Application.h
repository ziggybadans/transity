#pragma once

#include "core/ThreadPool.h"
#include "event/EventBus.h"
#include "render/ColorManager.h"
#include "ui/UIManager.h"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <memory>

class Game;
class UI;
class Renderer;

class Application {
public:
    Application();
    ~Application();
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
    std::unique_ptr<UIManager> _uiManager;

    sf::Clock _deltaClock;
    sf::Time _timeAccumulator;
    const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);

    bool _isWindowFocused = true;
};