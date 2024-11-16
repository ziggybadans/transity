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

    if (!m_worldMap) {
        std::cerr << "WorldMap is not set in Renderer." << std::endl;
        return false;
    }

    m_isInitialized = true;
    std::cout << "Renderer initialized successfully." << std::endl;
    return true;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!m_isInitialized) return;

    std::lock_guard<std::mutex> lock(m_renderMutex);

    // Apply the camera view
    camera.ApplyView(window);

    // Render various components
    RenderWorldMap(window, camera);
    m_placeAreaRenderer->Render(window, camera);
    RenderHoveredAreaName(window);

    // Restore the original view if necessary
    window.setView(window.getDefaultView());
}

void Renderer::Shutdown() {
    if (m_placeAreaRenderer) {
        m_placeAreaRenderer->Shutdown();
    }
    m_isInitialized = false;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    m_worldMap = map;
    if (m_placeAreaRenderer) {
        m_placeAreaRenderer->SetWorldMap(map);
    }
    std::cout << "WorldMap set in Renderer." << std::endl;
}

void Renderer::RenderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    if (!m_worldMap) return;

    // Store original view
    sf::View originalView = window.getView();
    window.setView(camera.GetView());

    // Calculate view bounds for culling
    sf::Vector2f viewCenter = camera.GetView().getCenter();
    sf::Vector2f viewSize = camera.GetView().getSize();
    sf::FloatRect cameraRect(
        viewCenter.x - viewSize.x / 2.0f,
        viewCenter.y - viewSize.y / 2.0f,
        viewSize.x,
        viewSize.y
    );

    const auto& landShapes = m_worldMap->GetMapData().GetLandShapes();
    for (const auto& shape : landShapes) {
        if (shape.getVertexCount() > 0 && cameraRect.intersects(shape.getBounds())) {
            window.draw(shape);
        }
    }

    // Restore original view
    window.setView(originalView);
}

void Renderer::RenderHoveredAreaName(sf::RenderWindow& window) {
    // Implementation for rendering hovered area name
}