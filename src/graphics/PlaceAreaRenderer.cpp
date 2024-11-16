#include "PlaceAreaRenderer.h"
#include <iostream>

PlaceAreaRenderer::PlaceAreaRenderer() {}

PlaceAreaRenderer::~PlaceAreaRenderer() {
    Shutdown();
}

bool PlaceAreaRenderer::Init() {
    if (!m_worldMap) {
        std::cerr << "WorldMap not set in PlaceAreaRenderer" << std::endl;
        return false;
    }
    return true;
}

void PlaceAreaRenderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!m_worldMap) return;

    // Calculate view bounds for culling
    sf::Vector2f viewCenter = camera.GetView().getCenter();
    sf::Vector2f viewSize = camera.GetView().getSize();
    sf::FloatRect cameraRect(
        viewCenter.x - viewSize.x / 2.0f,
        viewCenter.y - viewSize.y / 2.0f,
        viewSize.x,
        viewSize.y
    );

    const auto& placeAreas = m_worldMap->GetPlaceAreas();
    for (const auto& area : placeAreas) {
        // Only render if the area intersects with the camera view
        if (!cameraRect.intersects(area.bounds)) continue;

        // Draw the filled shape directly using the vertex array
        window.draw(area.filledShape);
        
        // Draw the outline
        if (area.outline.getVertexCount() > 0) {
            window.draw(area.outline);
        }
    }
}

void PlaceAreaRenderer::Shutdown() {
    m_worldMap.reset();
}

void PlaceAreaRenderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    m_worldMap = map;
    std::cout << "WorldMap set in PlaceAreaRenderer" << std::endl;
}
