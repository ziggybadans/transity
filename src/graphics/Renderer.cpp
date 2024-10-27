#include "Renderer.h"
#include <iostream>
#include <algorithm>

Renderer::Renderer() {}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& /*window*/) {
    isInitialized = true;

    if (!font.loadFromFile("assets/PTSans-Regular.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
        return false;
    }

    // Initialize hoveredAreaText
    hoveredAreaText.setFont(font);
    hoveredAreaText.setCharacterSize(28); // Adjust as needed
    hoveredAreaText.setFillColor(sf::Color::Black);

    return true;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    std::lock_guard<std::mutex> lock(renderMutex);

    renderWorldMap(window, camera);
    renderPlaceAreas(window, camera);
    renderHoveredAreaName(window);
}

void Renderer::Shutdown() {
    isInitialized = false;
    worldMap.reset();
}

void Renderer::renderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    if (worldMap) {
        worldMap->Render(window, camera);
    }
}

void Renderer::renderPlaceAreas(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& placeAreas = worldMap->GetPlaceAreas();

    // Apply camera view
    sf::View originalView = window.getView();
    window.setView(camera.GetView());

    // Get mouse position in world coordinates
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);

    // Reset hoveredAreaName
    hoveredAreaName.clear();

    for (const auto& area : placeAreas) {
        // Check if mouse is over the area
        sf::FloatRect bounds = area.filledShape.getBounds();
        if (bounds.contains(mouseWorldPos)) {
            // Use point-in-polygon test
            bool containsPoint = false;
            for (size_t i = 0; i < area.filledShape.getVertexCount(); i += 3) {
                // Create triangle
                sf::Vector2f p1 = area.filledShape[i].position;
                sf::Vector2f p2 = area.filledShape[i + 1].position;
                sf::Vector2f p3 = area.filledShape[i + 2].position;

                // Barycentric technique
                float denominator = ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
                if (denominator == 0.0f) continue; // Prevent division by zero

                float a = ((p2.y - p3.y) * (mouseWorldPos.x - p3.x) + (p3.x - p2.x) * (mouseWorldPos.y - p3.y)) / denominator;
                float b = ((p3.y - p1.y) * (mouseWorldPos.x - p3.x) + (p1.x - p3.x) * (mouseWorldPos.y - p3.y)) / denominator;
                float c = 1.0f - a - b;

                if (a >= 0 && b >= 0 && c >= 0) {
                    containsPoint = true;
                    break;
                }
            }
            if (containsPoint) {
                hoveredAreaName = area.name;

                // Create a temporary filled shape with yellow color
                sf::VertexArray highlightedFill = area.filledShape;
                for (size_t i = 0; i < highlightedFill.getVertexCount(); ++i) {
                    highlightedFill[i].color = sf::Color::Yellow; // Highlight fill color
                }
                window.draw(highlightedFill);

                // Draw the outline as normal
                window.draw(area.outline);
                continue;
            }
        }
        // Draw the regular filled shape and outline
        window.draw(area.filledShape);
        window.draw(area.outline);
    }

    // Restore the original view
    window.setView(originalView);
}

void Renderer::renderHoveredAreaName(sf::RenderWindow& window) {
    if (hoveredAreaName.empty()) return;

    hoveredAreaText.setString(hoveredAreaName);

    // Get window size
    sf::Vector2u windowSize = window.getSize();

    // Calculate text position: center-bottom with some padding
    sf::FloatRect textBounds = hoveredAreaText.getLocalBounds();
    float x = (windowSize.x - textBounds.width) / 2.0f;
    float y = windowSize.y - textBounds.height - 10.0f; // 10 pixels from bottom

    hoveredAreaText.setPosition(x, y);

    // Set view to default for UI rendering
    sf::View originalView = window.getView();
    window.setView(window.getDefaultView());

    window.draw(hoveredAreaText);

    // Restore original view
    window.setView(originalView);
}
