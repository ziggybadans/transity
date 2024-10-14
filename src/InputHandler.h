// src/InputHandler.h
#pragma once

#include <SFML/Graphics.hpp>

class InputHandler {
public:
    InputHandler(sf::View& view);

    // Process all user input events
    void processEvents(sf::RenderWindow& window);

private:
    sf::View& view;
    void handleCameraMovement();
    void wrapView();

    // Constants
    const float cameraSpeed = 10.0f;
};