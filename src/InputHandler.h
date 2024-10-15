// src/InputHandler.h
#pragma once

#include <SFML/Graphics.hpp>

class InputHandler {
public:
    InputHandler(sf::View& view, const sf::Vector2f& defaultViewSize);

    // Process all user input events
    void processEvents(sf::RenderWindow& window, sf::Time deltaTime);
    float minZoom = 0.5f;
    float maxZoom = 5.0f;

private:
    sf::View& view;
    sf::Vector2f defaultViewSize;
    void handleCameraMovement(sf::Time deltaTime);
    void handleZoom(sf::Event& event, sf::RenderWindow& window);

    // Constants
    const float cameraSpeed = 500.0f;

    const float zoomFactorIncrement = 1.1f;

};