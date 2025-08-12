// src/input/InputHandler.h
#pragma once

#include "../core/ServiceLocator.h"  // Include ServiceLocator
#include "../event/InputEvents.h"
#include "../event/LineEvents.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class InputHandler {
public:
    // Constructor now takes the ServiceLocator
    InputHandler(ServiceLocator &serviceLocator);

    // Method signatures are simplified
    void handleGameEvent(const sf::Event &event, sf::RenderWindow &window);
    void update(sf::Time dt);

private:
    ServiceLocator &_services;  // Reference to the ServiceLocator

    // Constants for zoom calculation
    float _zoomFactor;
    float _unzoomFactor;
};
