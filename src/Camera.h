#pragma once

#include <SFML/Graphics.hpp> // For sf::View, sf::RenderWindow
#include "InputHandler.h"    // For InputHandler

// Forward declaration for sf::Event if <SFML/Window/Event.hpp> is not fully included by Graphics.hpp for this context
namespace sf {
    class Event;
}

class Camera {
public:
    Camera();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(sf::Time dt);
    const sf::View& getView() const;
    void setInitialView(const sf::RenderWindow& window); // To set initial view based on window size

private:
    sf::View view;
    InputHandler m_inputHandler;
    // float cameraSpeed; // Moved to InputHandler
    // float zoomFactor; // Moved to InputHandler
    // float unzoomFactor; // Moved to InputHandler
};