#include "Game.h"
#include "Components.h" // Still needed for entity creation
#include "Renderer.h"   // For std::make_unique<Renderer>
#include <iostream>     // For std::cout (optional, can be removed if no logging)
#include <memory>       // For std::make_unique

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev")
    // oceanColor removed
     {
    camera.setInitialView(window); // Initialize camera view based on window
    window.setFramerateLimit(144);

    // Initialize Renderer
    m_renderer = std::make_unique<Renderer>(window);
    m_renderer->init(); // Call init to set up renderer-specific things like land, clear color

    // land initialization moved to Renderer::init()

    // Example entity creation - this part remains in Game logic
    // If land's position is needed for entities, it should be retrieved from renderer or a game config
    // For now, using a hardcoded position or a default from PositionComponent
    auto stationEntity = registry.create();
    // Assuming land's initial position was {50,50} and it's static, or game logic will place stations.
    // If Renderer::m_landShape's position is needed, Game would need a way to get it.
    // For simplicity, let's assume the station's position is independent for now.
    registry.emplace<PositionComponent>(stationEntity, sf::Vector2f{50.f, 50.f});
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
        m_renderer->render(registry, camera); // Delegate rendering to the Renderer instance
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        camera.handleEvent(event, window);
    }
}

void Game::update(sf::Time dt) {
    camera.update(dt);
    // Other game logic updates
}

// The old Game::render() method is now removed.
// Its logic has been moved to Renderer::render().