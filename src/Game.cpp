#include "Game.h"
#include "Renderer.h"
#include <memory>
#include <string>

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev")
    , m_entityFactory(registry)
     {
    m_renderer = std::make_unique<Renderer>(window);
    m_renderer->init();

    sf::Vector2f landCenter = m_renderer->getLandCenter();
    sf::Vector2f landSize = m_renderer->getLandSize();
    camera.setInitialView(window, landCenter, landSize);

    window.setFramerateLimit(144);
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

        std::optional<sf::Vector2f> stationPlacementPos = camera.handleEvent(event, window);
        int nextStationID = registry.alive() ? registry.size() : 0;
        if (stationPlacementPos) {
            m_entityFactory.createStation(stationPlacementPos.value(), "New Station" + std::to_string(nextStationID));
        }
    }
}

void Game::update(sf::Time dt) {
    camera.update(dt);
}