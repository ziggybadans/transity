#include "Renderer.h"
#include "../graphics/Camera.h"
#include <iostream>

Renderer::Renderer()
    : isInitialized(false), cityManager(nullptr) {}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& /*window*/, ThreadPool& /*threadPool*/) {
    // Initialize rendering resources if needed
    isInitialized = true;
    return true;
}

void Renderer::SetWorldMap(std::shared_ptr<WorldMap> map) {
    worldMap = map;
}

void Renderer::SetCityManager(CityManager* manager) {
    cityManager = manager;
}

void Renderer::SetCityCircleShape(std::shared_ptr<sf::CircleShape> shape) {
    cityShape = shape;
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    // Render WorldMap
    if (worldMap) {
        worldMap->Render(window, camera);
    }

    // Render cities
    if (cityManager && cityShape) {
        float zoomLevel = camera.GetZoomLevel();
        std::vector<City> citiesToRender = cityManager->GetCitiesToRender(zoomLevel);

        for (const auto& city : citiesToRender) {
            sf::CircleShape shape = *cityShape; // Copy the shape
            shape.setPosition(city.position);
            window.draw(shape);
        }
    }
}

void Renderer::Shutdown() {
    // Clean up rendering resources if needed
    isInitialized = false;
    worldMap.reset();
    cityManager = nullptr;
    cityShape.reset();
    // Reset other renderable components
}
