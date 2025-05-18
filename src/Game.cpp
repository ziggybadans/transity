#include "Game.h"
#include "Renderer.h"
#include <memory>

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev")
    , m_entityFactory(registry)
     {
    camera.setInitialView(window);
    window.setFramerateLimit(144);

    m_renderer = std::make_unique<Renderer>(window);
    m_renderer->init();

    m_entityFactory.createStation(sf::Vector2f(50.f, 50.f), "Central Station");
}

void Game::run() {
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();
        processEvents();
        update(dt);
        m_renderer->render(registry, camera);
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
}