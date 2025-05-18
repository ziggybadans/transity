#pragma once

#include <SFML/Graphics.hpp> // For sf::View, sf::RenderWindow, sf::Time
#include <SFML/Window.hpp>   // For sf::Event, sf::Mouse, sf::Keyboard

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