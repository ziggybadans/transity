#include "Game.h"
#include "Renderer.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev")
    , m_entityFactory(registry)
     {
    m_renderer = std::make_unique<Renderer>(window);
    m_renderer->init();

    sf::Vector2f landCenter = m_renderer->getLandCenter();
    sf::Vector2f landSize = m_renderer->getLandSize();
    camera.setInitialView(window, landCenter, landSize);

    ImGui::CreateContext();
    if (!ImGui::SFML::Init(window)) {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
    ImGui::StyleColorsDark();

    window.setFramerateLimit(144);
}

void Game::run() {
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        processEvents();

        ImGui::SFML::Update(window, dt);

        update(dt);

        ImGui::Begin("Debug Window");
        ImGui::Text("FPS: %.1f", 1.f / dt.asSeconds());
        ImGui::Text("Camera Position: (%.1f, %.1f)", camera.getView().getCenter().x, camera.getView().getCenter().y);
        ImGui::Text("Entity Count: %d", registry.alive());
        ImGui::End();

        m_renderer->render(registry, camera);

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown(window);
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
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