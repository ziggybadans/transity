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
    /* Map */
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

    /* Cities */
    for (City city : map.m_cities) {
        sf::CircleShape circleShape(city.radius);
        circleShape.setPosition(city.position);
        circleShape.setFillColor(sf::Color::Black);
        window.draw(circleShape);
    }

    /* Lines */
    for (auto& line : map.m_lines) {
        std::vector<City*> cities = line.GetCities();

        sf::VertexArray lineVertices(sf::Lines);
        for (size_t i = 0; i < cities.size() - 1; ++i) {
            lineVertices.append(sf::Vertex(cities[i]->position, line.GetColor()));
            lineVertices.append(sf::Vertex(cities[i + 1]->position, line.GetColor()));
        }
        
        window.draw(lineVertices);
    }
}

void Renderer::Shutdown() {
    m_isInitialized = false;
}