// src/InputHandler.cpp
#include "InputHandler.h"
#include "Utilities.h"

InputHandler::InputHandler(sf::View& viewRef, const sf::Vector2f& defaultSize)
    : view(viewRef), defaultViewSize(defaultSize)
{
}

void InputHandler::processEvents(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        // Close the window if the close button is pressed
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::MouseWheelScrolled) {
            handleZoom(event, window);
        }
    }

    // Handle camera movement based on keyboard input
    handleCameraMovement();

    // Wrap the view to ensure it stays within world boundaries
    wrapView();
}

void InputHandler::handleCameraMovement() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        view.move(-cameraSpeed, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        view.move(cameraSpeed, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        view.move(0, -cameraSpeed);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        view.move(0, cameraSpeed);
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
    view.move(beforeZoom - afterZoom);
}

void InputHandler::wrapView() {
    // Assuming world size is known or passed; otherwise, consider passing as parameters
    // For demonstration, we'll leave this empty or implement if world size is accessible
}
