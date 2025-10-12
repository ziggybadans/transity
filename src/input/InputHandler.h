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
    void handleMouseScroll(const sf::Event::MouseWheelScrolled &scrollData,
                           sf::RenderWindow &window);
    void handleMouseButtonPress(const sf::Event::MouseButtonPressed &pressData,
                                sf::RenderWindow &window);
    void handleMouseButtonRelease(const sf::Event::MouseButtonReleased &releaseData,
                                  sf::RenderWindow &window);
    void handleMouseMove(const sf::Event::MouseMoved &moveData, sf::RenderWindow &window);
    void handleKeyPress(const sf::Event::KeyPressed &keyData);
    void update(sf::Time dt);

private:
    EventBus &_eventBus;
    Camera &_camera;

    float _zoomFactor;
    float _unzoomFactor;
};