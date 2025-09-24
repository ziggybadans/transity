#pragma once

#include "event/EventBus.h"
#include "event/InputEvents.h"
#include "render/Camera.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class InputHandler {
public:
    InputHandler(EventBus &eventBus, Camera &camera);

    void handleGameEvent(const sf::Event &event, sf::RenderWindow &window);
    void update(sf::Time dt);

private:
    EventBus &_eventBus;
    Camera &_camera;

    float _zoomFactor;
    float _unzoomFactor;
};