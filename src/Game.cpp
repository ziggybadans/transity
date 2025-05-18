#include "Game.h"
// Components.h is no longer directly needed here as EntityFactory handles component specifics for stations
#include "Renderer.h"   // For std::make_unique<Renderer>
// iostream is not used in the current Game.cpp
#include <memory>       // For std::make_unique

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev")
    , m_entityFactory(registry) // Initialize EntityFactory with the registry
    // oceanColor removed
     {
    camera.setInitialView(window); // Initialize camera view based on window
    window.setFramerateLimit(144);

    // Initialize Renderer
    m_renderer = std::make_unique<Renderer>(window);
    m_renderer->init(); // Call init to set up renderer-specific things like land, clear color

    // land initialization moved to Renderer::init()

    // Create stations using EntityFactory
    m_entityFactory.createStation(sf::Vector2f(50.f, 50.f), "Central Station"); // Centered on land
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