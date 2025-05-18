#include "Game.h"
#include <iostream> // For std::optional in older C++ versions if needed, though SFML events should handle it.

Game::Game()
    : window(sf::VideoMode({800, 600}), "SFML window"),
      oceanColor(173, 216, 230) {
    camera.setInitialView(window); // Initialize camera view based on window

    // Initialize land
    land.setSize({100, 100});
    land.setFillColor(sf::Color::White);
    land.setPosition({50, 50});
}

void Game::run() {
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) { // Use traditional pollEvent
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        // Pass event to camera for handling zoom
        camera.handleEvent(event, window);
    }
}

void Game::update(sf::Time dt) {
    // Update camera (handles movement input)
    camera.update(dt);

    // Other game logic updates can go here
}

void Game::render() {
    window.setView(camera.getView());
    window.clear(oceanColor);
    window.draw(land);
    window.display();
}