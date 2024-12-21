#include "Renderer.h"
#include <iostream>
#include "../Debug.h"
#include "../Constants.h"

Renderer::Renderer() 
    : m_isInitialized(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init() {
    // Basic initialization that doesn't require a window
    m_isInitialized = false;
    return true;
}

bool Renderer::InitWithWindow(sf::RenderWindow& window) {
    if (m_isInitialized) return true;

    m_isInitialized = true;
    DEBUG_INFO("Renderer initialized successfully.");
    return true;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera, const Map& map) {
    if (!m_isInitialized) return;

    RenderMap(window, map);

    std::lock_guard<std::mutex> lock(m_renderMutex);
    camera.ApplyView(window);
}

void Renderer::RenderMap(sf::RenderWindow& window, const Map& map) {
    sf::RectangleShape tileShape(sf::Vector2f(Constants::TILE_SIZE, Constants::TILE_SIZE));
    for (size_t y = 0; y < map.GetSize(); ++y) {
        for (size_t x = 0; x < map.GetSize(); ++x) {
            tileShape.setPosition(x * Constants::TILE_SIZE, y * Constants::TILE_SIZE);

            if (map.GetTile(x, y) == 1) {
                tileShape.setFillColor(sf::Color::White);
            }

            window.draw(tileShape);
        }
    }

    for (City city : map.m_cities) {
        sf::CircleShape circleShape(city.radius);
        circleShape.setPosition(city.position);
        circleShape.setFillColor(sf::Color::Black);
        window.draw(circleShape);
    }
}

void Renderer::Shutdown() {
    m_isInitialized = false;
}