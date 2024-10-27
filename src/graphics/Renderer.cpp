// Renderer.cpp
#include "Renderer.h"
#include "../core/Station.h" // Include Station class
#include "../core/Line.h"     // Include Line class
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
    hoveredAreaText.setCharacterSize(40); // Adjust as needed
    hoveredAreaText.setFillColor(sf::Color::Black);

    return true;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    worldMap = map;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    std::lock_guard<std::mutex> lock(renderMutex);

    // Clear the window
    window.clear(sf::Color(174, 223, 246));

    // Apply camera view
    if (worldMap) {
        window.setView(camera.GetView());
    }

    // Render game elements via Renderer
    renderWorldMap(window, camera);
    renderPlaceAreas(window, camera);
    renderLines(window, camera);     // Render lines first
    renderStations(window, camera);  // Then render stations on top

    // Render UI elements if applicable
    renderHoveredAreaName(window);

    // Display the rendered frame
    window.display();
}

void Renderer::renderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    if (worldMap) {
        worldMap->Render(window, camera);
    }
}

void Renderer::renderPlaceAreas(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& placeAreas = worldMap->GetPlaceAreas();

    // Get the camera's view bounds
    sf::Vector2f viewCenter = camera.GetView().getCenter();
    sf::Vector2f viewSize = camera.GetView().getSize();
    sf::FloatRect cameraRect(viewCenter.x - viewSize.x / 2.0f,
        viewCenter.y - viewSize.y / 2.0f,
        viewSize.x,
        viewSize.y);

    // Get mouse position in world coordinates for hover functionality
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos, camera.GetView());

    // Reset hoveredAreaName
    hoveredAreaName.clear();

    // Calculate zoom factor
    float currentZoom = camera.GetZoomLevel();

    // Iterate through each place area
    for (const auto& area : placeAreas) {
        // Skip rendering based on zoom level
        float zoomThreshold = 0.0f;
        switch (area.category) {
        case PlaceCategory::City:
            zoomThreshold = 1.0f;
            break;
        case PlaceCategory::Town:
            zoomThreshold = 0.1f;
            break;
        case PlaceCategory::Suburb:
            zoomThreshold = 0.01f;
            break;
        default:
            break;
        }
        if (currentZoom > zoomThreshold) {
            continue; // Skip rendering this area
        }

        // Check if the area's bounding box intersects with the camera's view
        if (!cameraRect.intersects(area.bounds)) {
            continue; // Skip rendering if not in view
        }

        // Check if mouse is over the area
        if (area.bounds.contains(mouseWorldPos)) {
            // Perform point-in-polygon test
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
}

void Renderer::renderStations(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& stations = worldMap->GetStations();
    float currentZoom = camera.GetZoomLevel();

    for (const auto& station : stations) {
        station.Render(window, currentZoom);
    }
}

void Renderer::renderLines(sf::RenderWindow& window, const Camera& camera) {
    if (!worldMap) return;

    const auto& lines = worldMap->GetLines();
    float currentZoom = camera.GetZoomLevel();

    // Render existing lines
    for (const auto& line : lines) {
        line.Render(window, currentZoom);
    }

    // Render the current line being built
    const Line* currentLine = worldMap->GetCurrentLine();
    if (currentLine) {
        currentLine->Render(window, currentZoom); // Existing rendering of the line

        // Draw the preview line from the last node to the current mouse position
        if (!currentLine->GetNodes().empty()) {
            sf::Vector2f lastNode = currentLine->GetNodes().back();
            sf::Vector2f previewEnd = worldMap->currentMousePosition;

            sf::Vertex previewLine[] =
            {
                sf::Vertex(lastNode, sf::Color::Red), // Preview color
                sf::Vertex(previewEnd, sf::Color::Red)
            };

            window.draw(previewLine, 2, sf::Lines);
        }
    }
}

void Renderer::renderHoveredAreaName(sf::RenderWindow& window) {
    if (hoveredAreaName.empty()) return;

    hoveredAreaText.setString(hoveredAreaName);

    // Get window size
    sf::Vector2u windowSize = window.getSize();

    // Calculate text position: center-bottom with some padding
    sf::FloatRect textBounds = hoveredAreaText.getLocalBounds();
    float x = (windowSize.x - textBounds.width) / 2.0f;
    float y = windowSize.y - textBounds.height - 40.0f; // 40 pixels from bottom

    hoveredAreaText.setPosition(x, y);

    // Set view to default for UI rendering
    sf::View originalView = window.getView();
    window.setView(window.getDefaultView());

    window.draw(hoveredAreaText);

    // Restore original view
    window.setView(originalView);
}

void Renderer::Shutdown() {
    isInitialized = false;
    worldMap.reset();
}
