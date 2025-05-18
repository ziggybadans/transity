#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class InputHandler {
public:
    InputHandler();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window, sf::View& view);
    void update(sf::Time dt, sf::View& view);

private:
    float cameraSpeed;
    float zoomFactor;
    float unzoomFactor;
};