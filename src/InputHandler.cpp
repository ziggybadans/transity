#include "InputHandler.h"

// Constructor for InputHandler, initializing input-related variables
InputHandler::InputHandler()
    : cameraSpeed(200.0f), zoomFactor(0.9f), unzoomFactor(1.0f / zoomFactor) {
    // Constructor can be empty if all initialization is done in the initializer list
}

// Handles mouse wheel scroll events for zooming
void InputHandler::handleEvent(const sf::Event& event, const sf::RenderWindow& window, sf::View& view) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPosBeforeZoom = window.mapPixelToCoords(mousePixelPos, view);

            if (event.mouseWheelScroll.delta > 0) { // Zoom in
                view.zoom(zoomFactor);
            } else if (event.mouseWheelScroll.delta < 0) { // Zoom out
                view.zoom(unzoomFactor);
            }

            sf::Vector2f worldPosAfterZoom = window.mapPixelToCoords(mousePixelPos, view);
            sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
            view.move(offset);
        }
    }
}

// Handles keyboard input for camera movement
void InputHandler::update(sf::Time dt, sf::View& view) {
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        movement.y -= cameraSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        movement.y += cameraSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        movement.x -= cameraSpeed * dt.asSeconds();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        movement.x += cameraSpeed * dt.asSeconds();

    if (movement.x != 0.f || movement.y != 0.f) {
        view.move(movement);
    }
}