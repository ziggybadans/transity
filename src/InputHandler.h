#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <optional>

class InputHandler {
public:
    InputHandler();
    std::optional<sf::Vector2f> handleEvent(const sf::Event& event, const sf::RenderWindow& window, sf::View& view);
    void update(sf::Time dt, sf::View& view);

private:
    float cameraSpeed;
    float zoomFactor;
    float unzoomFactor;
};