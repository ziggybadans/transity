// Renderer.cpp
#include "Renderer.h"
#include "PlaceAreaRenderer.h"
#include <iostream>

Renderer::Renderer() 
    : isInitialized(false),
      placeAreaRenderer(std::make_unique<PlaceAreaRenderer>())
{
    // Additional initialization code if needed
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Init(sf::RenderWindow& window) {
    if (isInitialized) return true;

    // Initialize specialized renderers
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
    if (placeAreaRenderer) placeAreaRenderer->SetWorldMap(map);
}

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    std::lock_guard<std::mutex> lock(renderMutex);

    // Clear the window with a background color
    window.clear(sf::Color(174, 223, 246)); // Sky-blue color

    // Render each component using specialized renderers
    placeAreaRenderer->Render(window, camera);

    // Optionally render hovered area name
    renderHoveredAreaName(window);

    // Display the rendered frame
    // Note: In the main game loop, window.display() is called, so it's not called here to avoid multiple displays
}

void Renderer::Shutdown() {
    if (placeAreaRenderer) {
        placeAreaRenderer->Shutdown();
    }
    isInitialized = false;
}

void Renderer::renderHoveredAreaName(sf::RenderWindow& window) {
    // Implementation for rendering hovered area name if needed
    // Since lines and stations are removed, ensure this only pertains to place areas
}