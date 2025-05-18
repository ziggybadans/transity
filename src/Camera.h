#pragma once

#include <SFML/Graphics.hpp>
#include "InputHandler.h"

namespace sf {
    class Event;
}

class Camera {
public:
    Camera();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(sf::Time dt);
    const sf::View& getView() const;
    void setInitialView(const sf::RenderWindow& window);

private:
    sf::View view;
    InputHandler m_inputHandler;
};