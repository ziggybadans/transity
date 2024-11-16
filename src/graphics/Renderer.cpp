// Renderer.cpp
#include "Renderer.h"
#include "LineRenderer.h"
#include "StationRenderer.h"
#include "PlaceAreaRenderer.h"
#include <iostream>

Renderer::Renderer() 
    : isInitialized(false),
      lineRenderer(std::make_unique<LineRenderer>()),
      stationRenderer(std::make_unique<StationRenderer>()),
      placeAreaRenderer(std::make_unique<PlaceAreaRenderer>()) {
    // Additional initialization code if needed
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& window) {
    if (isInitialized) return true;

    // Initialize specialized renderers
    lineRenderer = std::make_unique<LineRenderer>();
    stationRenderer = std::make_unique<StationRenderer>();
    placeAreaRenderer = std::make_unique<PlaceAreaRenderer>();

    // Initialize each specialized renderer
    if (!lineRenderer->Init()) {
        std::cerr << "Failed to initialize LineRenderer." << std::endl;
        return false;
    }
    if (!stationRenderer->Init()) {
        std::cerr << "Failed to initialize StationRenderer." << std::endl;
        return false;
    }
    if (!placeAreaRenderer->Init()) {
        std::cerr << "Failed to initialize PlaceAreaRenderer." << std::endl;
        return false;
    }

    isInitialized = true;
    return true;
}

void Renderer::SetWorldMap(const std::shared_ptr<WorldMap>& map) {
    std::lock_guard<std::mutex> lock(renderMutex);
    worldMap = map;

    // Pass the world map to specialized renderers
    if (lineRenderer) lineRenderer->SetWorldMap(map);
    if (stationRenderer) stationRenderer->SetWorldMap(map);
    if (placeAreaRenderer) placeAreaRenderer->SetWorldMap(map);
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    std::lock_guard<std::mutex> lock(renderMutex);

    // Clear the window with a background color
    window.clear(sf::Color(174, 223, 246)); // Sky-blue color

    // Render each component using specialized renderers
    placeAreaRenderer->Render(window, camera);
    lineRenderer->Render(window, camera);
    stationRenderer->Render(window, camera);

    // Optionally render hovered area name
    renderHoveredAreaName(window);

    // Display the rendered frame
    window.display();
}

void Renderer::Shutdown() {
    if (!isInitialized) return;

    // Shutdown specialized renderers
    if (lineRenderer) lineRenderer->Shutdown();
    if (stationRenderer) stationRenderer->Shutdown();
    if (placeAreaRenderer) placeAreaRenderer->Shutdown();

    isInitialized = false;
}

void Renderer::renderWorldMap(sf::RenderWindow& window, const Camera& camera) {
    // Delegate to specialized renderers
}

void Renderer::renderHoveredAreaName(sf::RenderWindow& window) {
    // Implement if needed
}