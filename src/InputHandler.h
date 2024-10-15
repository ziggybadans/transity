// src/InputHandler.h
#pragma once

#include <SFML/Graphics.hpp>

class InputHandler {
public:
    InputHandler(sf::View& view, const sf::Vector2f& defaultViewSize);

    // Process all user input events
    void processEvents(sf::RenderWindow& window);

private:
    sf::View& view;
    void handleCameraMovement();
    void handleZoom(sf::Event& event, sf::RenderWindow& window);
    void wrapView();

    // Constants
    const float cameraSpeed = 10.0f;

    const float zoomFactorIncrement = 1.1f;
    const float minZoom = 0.5f;
    const float maxZoom = 5.0f;

    sf::Vector2f defaultViewSize;
};