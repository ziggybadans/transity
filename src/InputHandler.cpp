// src/InputHandler.cpp
#include "InputHandler.h"

#include <imgui.h>
#include <imgui-SFML.h>

InputHandler::InputHandler(sf::View& viewRef, const sf::Vector2f& defaultSize)
    : view(viewRef), defaultViewSize(defaultSize)
{
}

void InputHandler::processEvents(sf::RenderWindow& window, sf::Time deltaTime) {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        // Handle window close event
        if (event.type == sf::Event::Closed) {
            closeRequested = true;
        }

        if (event.type == sf::Event::MouseWheelScrolled) {
            handleZoom(event, window);
        }
    }

    // Handle camera movement based on keyboard input
    handleCameraMovement(deltaTime);
}

void InputHandler::handleCameraMovement(sf::Time deltaTime) {
    float deltaSeconds = deltaTime.asSeconds();

    // Adjust camera speed based on current zoom level
    float currentZoom = view.getSize().x / defaultViewSize.x;
    float adjustedCameraSpeed = cameraSpeed * currentZoom;

    sf::Vector2f movement(0.0f, 0.0f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        movement.x -= adjustedCameraSpeed * deltaSeconds;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        movement.x += adjustedCameraSpeed * deltaSeconds;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        movement.y -= adjustedCameraSpeed * deltaSeconds;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        movement.y += adjustedCameraSpeed * deltaSeconds;
    }

    if (movement != sf::Vector2f(0.0f, 0.0f)) {
        view.move(movement);
    }
}

void InputHandler::handleZoom(sf::Event& event, sf::RenderWindow& window)
{
    // Get the mouse position relative to the window
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    // Convert to world coordinates
    sf::Vector2f beforeZoom = window.mapPixelToCoords(pixelPos, view);

    if (event.mouseWheelScroll.delta > 0) {
        // Zoom in
        if (view.getSize().x > minZoom * defaultViewSize.x &&
            view.getSize().y > minZoom * defaultViewSize.y) {
            view.zoom(1.0f / zoomFactorIncrement);
        }
    }
    else if (event.mouseWheelScroll.delta < 0) {
        // Zoom out
        if (view.getSize().x < maxZoom * defaultViewSize.x &&
            view.getSize().y < maxZoom * defaultViewSize.y) {
            view.zoom(zoomFactorIncrement);
        }
    }

    // After zooming, get the new world coordinates
    sf::Vector2f afterZoom = window.mapPixelToCoords(pixelPos, view);
    // Adjust the view center to zoom towards the mouse position
    if (beforeZoom != afterZoom) {
        view.move(beforeZoom - afterZoom);
    }
}

void InputHandler::setMinZoom(float newMinZoom) {
    minZoom = newMinZoom;
}

void InputHandler::setMaxZoom(float newMaxZoom) {
    maxZoom = newMaxZoom;
}

bool InputHandler::shouldClose() const {
    return closeRequested;
}
