#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp> // For sf::Event and sf::Mouse

class Camera {
public:
    Camera();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(sf::Time dt);
    const sf::View& getView() const;
    void setInitialView(const sf::RenderWindow& window); // To set initial view based on window size

private:
    sf::View view;
    float cameraSpeed;
    float zoomFactor;
    float unzoomFactor;
};