#include "LineRenderer.h"
#include <iostream>

LineRenderer::LineRenderer() {}

LineRenderer::~LineRenderer() {
    Shutdown();
}

bool LineRenderer::Init() {
    if (!font.loadFromFile("assets/PTSans-Regular.ttf")) {
        std::cerr << "Failed to load font in LineRenderer." << std::endl;
        return false;
    }

    hoveredLineText.setFont(font);
    hoveredLineText.setCharacterSize(14);
    hoveredLineText.setFillColor(sf::Color::Black);

    return true;
}

void LineRenderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

void LineRenderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    renderLines(window, camera);
}

void LineRenderer::Shutdown() {
    // Clean up resources if any
}

void LineRenderer::renderLines(sf::RenderWindow& window, const Camera& camera) {
    const auto& lines = worldMap->GetLines();
    for (const auto& linePtr : lines) {
        if (!linePtr->IsActive()) continue;

        sf::VertexArray vertices(sf::LinesStrip, linePtr->GetSplinePoints().size());
        for (size_t i = 0; i < linePtr->GetSplinePoints().size(); ++i) {
            vertices[i].position = linePtr->GetSplinePoints()[i];
            vertices[i].color = linePtr->GetColor();
        }
        window.draw(vertices);

        // Optionally render line name or other details
    }
}
