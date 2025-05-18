#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include "InputHandler.h"

namespace sf {
    class Event;
}

class Camera {
public:
    Camera();
    std::optional<sf::Vector2f> handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(sf::Time dt);
    const sf::View& getView() const;
    void setInitialView(const sf::RenderWindow& window, const sf::Vector2f& landCenter, const sf::Vector2f& landSize);

private:
    sf::View view;
    InputHandler m_inputHandler;
};