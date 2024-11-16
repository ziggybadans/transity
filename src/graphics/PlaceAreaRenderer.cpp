#include "PlaceAreaRenderer.h"
#include <iostream>

PlaceAreaRenderer::PlaceAreaRenderer() {}

PlaceAreaRenderer::~PlaceAreaRenderer() {
    Shutdown();
}

bool PlaceAreaRenderer::Init() {
    return true;
}

void PlaceAreaRenderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!m_worldMap) return;
    RenderPlaceAreas(window, camera);
}

void PlaceAreaRenderer::Shutdown() {
    // Clean up resources if needed
}

void PlaceAreaRenderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    m_worldMap = map;
}

void PlaceAreaRenderer::RenderPlaceAreas(sf::RenderWindow& window, const Camera& camera) {
    const auto& placeAreas = m_worldMap->GetPlaceAreas();
    
    for (const auto& area : placeAreas) {
        // Render filled shape
        sf::ConvexShape shape;
        shape.setPointCount(area.filledShape.getVertexCount());
        for (size_t i = 0; i < area.filledShape.getVertexCount(); ++i) {
            shape.setPoint(i, area.filledShape[i].position);
        }
        shape.setFillColor(sf::Color(100, 100, 100, 100));
        window.draw(shape);

        // Render outline if present
        if (area.outline.getVertexCount() > 0) {
            sf::ConvexShape outlineShape;
            outlineShape.setPointCount(area.outline.getVertexCount());
            for (size_t i = 0; i < area.outline.getVertexCount(); ++i) {
                outlineShape.setPoint(i, area.outline[i].position);
            }
            outlineShape.setFillColor(sf::Color::Transparent);
            outlineShape.setOutlineThickness(1.0f);
            outlineShape.setOutlineColor(sf::Color::Black);
            window.draw(outlineShape);
        }
    }
}
