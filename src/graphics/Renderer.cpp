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

        // Determine zoom level based on currentZoom
        int cityZoomLevel = 0;
        if (currentZoom >= 2.0f) {
            cityZoomLevel = 1;
        }
        else if (currentZoom >= 1.0f) {
            cityZoomLevel = 2;
        }
        else if (currentZoom >= 0.5f) {
            cityZoomLevel = 3;
        }
        else {
            cityZoomLevel = 4;
        }

        // Define base sizes
        float baseCircleRadius = 8.0f;

        // Scaling factors
        float scaledCircleRadius = baseCircleRadius * currentZoom * 2;

        // Set up circle shape
        sf::CircleShape circle;
        circle.setFillColor(sf::Color::White);
        circle.setOutlineColor(sf::Color::Black);

        sf::View originalView = window.getView();
        window.setView(camera.GetView());

        for (const auto& city : cities) {
            if (city.zoomLevel <= cityZoomLevel) {
                // Update circle
                circle.setRadius(scaledCircleRadius);
                circle.setOrigin(scaledCircleRadius, scaledCircleRadius);
                circle.setOutlineThickness(8.0f * currentZoom);
                circle.setPosition(city.position);

                window.draw(circle);
            }
        }

        window.setView(originalView);
    }
}

void Renderer::Shutdown() {
    // Clean up rendering resources if needed
    isInitialized = false;
    worldMap.reset();
}
