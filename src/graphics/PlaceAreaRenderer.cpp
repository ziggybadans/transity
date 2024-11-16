#include "PlaceAreaRenderer.h"
#include <iostream>

PlaceAreaRenderer::PlaceAreaRenderer() {}

PlaceAreaRenderer::~PlaceAreaRenderer() {
    Shutdown();
}

bool PlaceAreaRenderer::Init() {
    // Initialize if needed
    return true;
}

void PlaceAreaRenderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

void PlaceAreaRenderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    renderPlaceAreas(window, camera);
}

void PlaceAreaRenderer::Shutdown() {
    // Clean up resources if any
}

void PlaceAreaRenderer::renderPlaceAreas(sf::RenderWindow& window, const Camera& camera) {
    const auto& placeAreas = worldMap->GetPlaceAreas();
    for (const auto& area : placeAreas) {
        // Create a ConvexShape from the filledShape VertexArray
        sf::ConvexShape shape;
        shape.setPointCount(area.filledShape.getVertexCount());
        for (size_t i = 0; i < area.filledShape.getVertexCount(); ++i) {
            shape.setPoint(i, area.filledShape[i].position);
        }
        shape.setFillColor(sf::Color(100, 100, 100, 100)); // Semi-transparent gray
        window.draw(shape);

        // Optionally, you can also render the outline if needed
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
