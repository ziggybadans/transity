#include "Game.h"
#include "Renderer.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Logger.h"
#include <memory>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

Game::Game()
    : window(sf::VideoMode({800, 600}), "Transity Predev")
    , m_entityFactory(registry)
     {
    LOG_INFO("Game", "Game instance creating.");
    window.setFramerateLimit(144);
    LOG_INFO("Game", "Game instance created successfully.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");
        m_renderer = std::make_unique<Renderer>(window);
    m_renderer->init();

    sf::Vector2f landCenter = m_renderer->getLandCenter();
    sf::Vector2f landSize = m_renderer->getLandSize();
    camera.setInitialView(window, landCenter, landSize);

    ImGui::CreateContext();
    if (!ImGui::SFML::Init(window)) {
        LOG_FATAL("Game", "Failed to initialize ImGui-SFML");
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }
    ImGui::StyleColorsDark();
    LOG_INFO("Game", "Game initialization completed.");
}

void Game::run() {
    LOG_INFO("Game", "Starting game loop.");
    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();
        LOG_TRACE("Game", "Delta time: %f seconds", dt.asSeconds());

        processEvents();

        ImGui::SFML::Update(window, dt);

        update(dt);
        LOG_TRACE("Game", "Game logic updated.");

        ImGui::Begin("Debug Window");
        ImGui::Text("FPS: %.1f", 1.f / dt.asSeconds());
        ImGui::Text("Camera Position: (%.1f, %.1f)", camera.getView().getCenter().x, camera.getView().getCenter().y);
        ImGui::Text("Entity Count: %d", registry.alive());
        ImGui::End();

        m_renderer->render(registry, camera);
        LOG_TRACE("Game", "Frame rendered.");

        ImGui::SFML::Render(window);

        window.display();
    }
    LOG_INFO("Game", "Game loop ended.");
    ImGui::SFML::Shutdown(window);
    LOG_INFO("Game", "ImGui shutdown.");
}

void Game::processEvents() {
    LOG_TRACE("Game", "Processing events.");
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed) {
            LOG_INFO("Game", "Window close event received.");
            window.close();
        }

        std::optional<sf::Vector2f> stationPlacementPos = camera.handleEvent(event, window);
        int nextStationID = registry.alive() ? registry.size() : 0;
        if (stationPlacementPos) {
            LOG_DEBUG("Game", "Station placement event at (%.1f, %.1f)", stationPlacementPos.value().x, stationPlacementPos.value().y);
            m_entityFactory.createStation(stationPlacementPos.value(), "New Station" + std::to_string(nextStationID));
        }
    }
}

void Game::update(sf::Time dt) {
    camera.update(dt);
}