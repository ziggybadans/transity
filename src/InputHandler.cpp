// src/InputHandler.cpp
#include "InputHandler.h"
#include "Utilities.h"

InputHandler::InputHandler(sf::View& viewRef)
    : view(viewRef)
{
}

void InputHandler::processEvents(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        // Close the window if the close button is pressed
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }

    // Handle camera movement based on keyboard input
    handleCameraMovement();

    // Wrap the view to ensure it stays within world boundaries
    wrapView();
}

void InputHandler::handleCameraMovement() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        view.move(-cameraSpeed, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        view.move(cameraSpeed, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        view.move(0, -cameraSpeed);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        view.move(0, cameraSpeed);
    }
}

void InputHandler::wrapView() {
    // Assuming world size is known or passed; otherwise, consider passing as parameters
    // For demonstration, we'll leave this empty or implement if world size is accessible
}
