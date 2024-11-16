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

    // Clear window with sky-blue background
    window.clear(sf::Color(174, 223, 246));

    // Render the world map first (base layer)
    RenderWorldMap(window, camera);

    // Render place areas on top
    m_placeAreaRenderer->Render(window, camera);
    
    // Render UI elements last
    RenderHoveredAreaName(window);
}

void Renderer::Shutdown() {
    if (m_placeAreaRenderer) {
        m_placeAreaRenderer->Shutdown();
    }
    m_isInitialized = false;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    m_worldMap = map;
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

    // Draw land shapes with culling
    const auto& landShapes = m_worldMap->GetMapData().GetLandShapes();
    for (const auto& shape : landShapes) {
        // Calculate bounds manually for vertex array
        sf::FloatRect shapeBounds;
        if (shape.getVertexCount() > 0) {
            float minX = shape[0].position.x;
            float minY = shape[0].position.y;
            float maxX = minX;
            float maxY = minY;
            
            for (size_t i = 1; i < shape.getVertexCount(); ++i) {
                const auto& pos = shape[i].position;
                minX = std::min(minX, pos.x);
                minY = std::min(minY, pos.y);
                maxX = std::max(maxX, pos.x);
                maxY = std::max(maxY, pos.y);
            }
            
            shapeBounds = sf::FloatRect(minX, minY, maxX - minX, maxY - minY);
            
            // Only draw if in view
            if (cameraRect.intersects(shapeBounds)) {
                window.draw(shape);
            }
        }
    }

    // Restore original view
    window.setView(originalView);
}

void Renderer::RenderHoveredAreaName(sf::RenderWindow& window) {
    // Implementation for rendering hovered area name
}