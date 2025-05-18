#include "InputHandler.h"
#include "Logger.h"
#include <iostream>

InputHandler::InputHandler()
    : cameraSpeed(200.0f), zoomFactor(0.9f), unzoomFactor(1.0f / zoomFactor) {
    LOG_INFO("Input", "InputHandler created.");
}

std::optional<sf::Vector2f> InputHandler::handleEvent(const sf::Event& event, const sf::RenderWindow& window, sf::View& view) {
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            LOG_DEBUG("Input", "Mouse wheel scrolled: delta %.1f", event.mouseWheelScroll.delta);
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPosBeforeZoom = window.mapPixelToCoords(mousePixelPos, view);

            if (event.mouseWheelScroll.delta > 0) {
                view.zoom(zoomFactor);
                LOG_TRACE("Input", "Zoomed in.");
            } else if (event.mouseWheelScroll.delta < 0) {
                view.zoom(unzoomFactor);
                LOG_TRACE("Input", "Zoomed out.");
            }

            sf::Vector2f worldPosAfterZoom = window.mapPixelToCoords(mousePixelPos, view);
            sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
            view.move(offset);
            LOG_TRACE("Input", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x, offset.y);
        }
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePixelPos, view);
            LOG_DEBUG("Input", "Right mouse button pressed at screen ( %d, %d ), world (%.1f, %.1f)", mousePixelPos.x, mousePixelPos.y, worldPos.x, worldPos.y);
            return worldPos;
        }
    }

    return std::nullopt;
}

void InputHandler::update(sf::Time dt, sf::View& view) {
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        movement.y -= cameraSpeed * dt.asSeconds();
        LOG_TRACE("Input", "Key W pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        movement.y += cameraSpeed * dt.asSeconds();
        LOG_TRACE("Input", "Key S pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        movement.x -= cameraSpeed * dt.asSeconds();
        LOG_TRACE("Input", "Key A pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        movement.x += cameraSpeed * dt.asSeconds();
        LOG_TRACE("Input", "Key D pressed.");
    }

    if (movement.x != 0.f || movement.y != 0.f) {
        view.move(movement);
        LOG_TRACE("Input", "View moved by (%.1f, %.1f) due to keyboard input.", movement.x, movement.y);
    }
}