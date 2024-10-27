#include "Renderer.h"
#include "../graphics/Camera.h"
#include <iostream>
#include <algorithm>

Renderer::Renderer()
    : isInitialized(false) {} // Initialize base radius
// You can adjust the default base radius as needed

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& /*window*/, ThreadPool& /*threadPool*/) {
    isInitialized = true;

    if (!font.loadFromFile("assets/PTSans-Regular.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
        return false;
    }

    // Initialize hoveredCityText
    hoveredCityText.setFont(font);
    hoveredCityText.setCharacterSize(28); // Adjust as needed
    hoveredCityText.setFillColor(sf::Color::Black);
    // Position will be set dynamically based on window size

    return true;
}

void Renderer::SetWorldMap(std::shared_ptr<WorldMap> map) {
    worldMap = map;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    // Render WorldMap
    if (worldMap) {
        worldMap->Render(window, camera);
    }

    // Render Cities
    if (worldMap) {
        const auto& cities = worldMap->GetCities();

        float currentZoom = camera.GetZoomLevel();

        int cityZoomLevel = 0;
        if (currentZoom <= 1.0f && currentZoom > 0.5f) {
            cityZoomLevel = 1;
        }
        else if (currentZoom <= 0.5f && currentZoom > 0.1f) {
            cityZoomLevel = 2;
        }
        else if (currentZoom <= 0.1f && currentZoom > 0.005f) {
            cityZoomLevel = 3;
        }
        else if (currentZoom <= 0.005f) {
            cityZoomLevel = 4;
        }

        // Define base sizes
        float baseCircleRadius = 8.0f;

        // Set up circle shape
        sf::CircleShape circle;
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color::Black);

        sf::View originalView = window.getView();
        window.setView(camera.GetView());

        // Get mouse position in window coordinates
        sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
        // Convert to world coordinates
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos, camera.GetView());

        // Reset hoveredCityName
        hoveredCityName = "";

        // Iterate through cities to render and detect hover
        for (const auto& city : cities) {
            if (city.zoomLevel <= cityZoomLevel) {
                // Adjust circle size based on city's zoomLevel
                float citySizeFactor = 1.0f;
                switch (city.zoomLevel) {
                case 1:
                    citySizeFactor = 1.0f; // Largest cities
                    break;
                case 2:
                    citySizeFactor = 0.75f; // Medium cities
                    break;
                case 3:
                    citySizeFactor = 0.5f; // Smaller cities
                    break;
                case 4:
                    citySizeFactor = 0.25f; // Smallest cities
                    break;
                default:
                    citySizeFactor = 0.1f;
                    break;
                }

                // Scaling factors
                float scaledCircleRadius = std::min((baseCircleRadius * currentZoom * citySizeFactor + (8 * currentZoom)), 6.0f);

                // Update circle properties
                circle.setRadius(scaledCircleRadius);
                circle.setOrigin(scaledCircleRadius, scaledCircleRadius);
                circle.setOutlineThickness(std::min(4.0f * currentZoom, 4.0f));
                circle.setPosition(city.position);

                // Check if mouse is over this city
                float dx = mouseWorldPos.x - city.position.x;
                float dy = mouseWorldPos.y - city.position.y;
                float distanceSquared = dx * dx + dy * dy;

                if (distanceSquared <= scaledCircleRadius * scaledCircleRadius) {
                    hoveredCityName = city.name;
                    // Optional: Highlight the hovered city
                    circle.setFillColor(sf::Color::Yellow); // Highlight color
                }

                window.draw(circle);

                // Reset circle color for next iteration
                circle.setFillColor(sf::Color::White);
            }
        }

        // Restore the original view
        window.setView(originalView);
    }

    // Draw the hovered city name at the bottom center
    if (!hoveredCityName.empty()) {
        hoveredCityText.setString(hoveredCityName);

        // Get window size
        sf::Vector2u windowSize = window.getSize();

        // Calculate text position: center-bottom with some padding
        sf::FloatRect textBounds = hoveredCityText.getLocalBounds();
        float x = (windowSize.x - textBounds.width) / 2.0f;
        float y = windowSize.y - textBounds.height - 10.0f; // 10 pixels from bottom

        hoveredCityText.setPosition(x, y);

        // Set view to default for UI rendering
        sf::View originalView = window.getView();
        window.setView(window.getDefaultView());

        window.draw(hoveredCityText);

        // Restore original view
        window.setView(originalView);
    }
}

void Renderer::Shutdown() {
    // Clean up rendering resources if needed
    isInitialized = false;
    worldMap.reset();
}
