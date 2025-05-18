#include "Camera.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

Camera::Camera()
    : cameraSpeed(200.0f), zoomFactor(0.9f), unzoomFactor(1.0f / zoomFactor) {
    // Initialize view with default values, can be adjusted by setInitialView
    view.setSize({800, 600});
    view.setCenter({400, 300});
}

void Camera::setInitialView(const sf::RenderWindow& window) {
    sf::Vector2u windowSize = window.getSize();
    view.setSize(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y));
    view.setCenter(static_cast<float>(windowSize.x) / 2.f, static_cast<float>(windowSize.y) / 2.f);
}

void Camera::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        // Check if it's the vertical wheel (common for zooming)
        // SFML versions prior to 2.6 used sf::Mouse::VerticalWheel
        // For broader compatibility, we can often just check event.mouseWheelScroll.delta
        // and assume vertical scroll for zoom, or specifically check the wheel if needed.
        // The original code checked scrolledEvent->wheel == sf::Mouse::Wheel::Vertical
        // which implies we should check for the vertical wheel.
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) { // Use older enum
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

void Camera::update(sf::Time dt) {
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

const sf::View& Camera::getView() const {
    return view;
}