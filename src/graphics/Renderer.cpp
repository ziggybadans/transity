// Renderer.cpp
#include "Renderer.h"
#include "PlaceAreaRenderer.h"
#include <iostream>

Renderer::Renderer() 
    : m_isInitialized(false),
      m_placeAreaRenderer(std::make_unique<PlaceAreaRenderer>())
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& window) {
    if (m_isInitialized) return true;

    if (!m_placeAreaRenderer->Init()) {
        std::cerr << "Failed to initialize PlaceAreaRenderer." << std::endl;
        return false;
    }

    m_isInitialized = true;
    return true;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!m_isInitialized) return;

    std::lock_guard<std::mutex> lock(m_renderMutex);

    // Clear window with sky-blue background
    window.clear(sf::Color(174, 223, 246));

    m_placeAreaRenderer->Render(window, camera);
    RenderHoveredAreaName(window);
}

void Renderer::Shutdown() {
    if (m_placeAreaRenderer) {
        m_placeAreaRenderer->Shutdown();
    }
    m_isInitialized = false;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    std::lock_guard<std::mutex> lock(m_renderMutex);
    m_worldMap = map;

    if (m_placeAreaRenderer) {
        m_placeAreaRenderer->SetWorldMap(map);
    }
}

void Renderer::RenderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    // Implementation for rendering world map
}

void Renderer::RenderHoveredAreaName(sf::RenderWindow& window) {
    // Implementation for rendering hovered area name
}