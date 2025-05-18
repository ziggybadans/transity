#include "Game.h"
#include "Components.h"
#include <iostream> // For std::optional in older C++ versions if needed, though SFML events should handle it.

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev"),
      oceanColor(173, 216, 230) {
    camera.setInitialView(window); // Initialize camera view based on window
    window.setFramerateLimit(144);

    // Initialize land
    land.setSize({100, 100});
    land.setFillColor(sf::Color::White);
    land.setOrigin(land.getSize() / 2.0f);
    land.setPosition({50, 50});

    auto stationEntity = registry.create();
    registry.emplace<PositionComponent>(stationEntity, land.getPosition());
    registry.emplace<NameComponent>(stationEntity, "Station");
    registry.emplace<StationTag>(stationEntity);

    auto& renderable = registry.emplace<RenderableComponent>(stationEntity);
    renderable.shape.setRadius(5);
    renderable.shape.setFillColor(sf::Color::Green);
    renderable.shape.setOrigin(renderable.shape.getRadius(), renderable.shape.getRadius());
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

    auto view = registry.view<PositionComponent, RenderableComponent>();
    for (auto entity : view) {
        auto& position = view.get<PositionComponent>(entity);
        auto& renderable = view.get<RenderableComponent>(entity);

        renderable.shape.setPosition(position.coordinates);
        window.draw(renderable.shape);
    }

    window.display();
}