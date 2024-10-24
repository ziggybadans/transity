#include "Renderer.h"
#include "../graphics/Camera.h"
#include <iostream>

Renderer::Renderer()
    : isInitialized(false) {}

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

void Renderer::Render(sf::RenderWindow& window, const Camera& camera) {
    if (!isInitialized) return;

    // Render WorldMap
    if (worldMap) {
        worldMap->Render(window, camera);
    }

    // Render other game elements here
    // Example:
    // for (const auto& entity : entities) {
    //     window.draw(entity->getSprite());
    // }
}

void Renderer::Shutdown() {
    // Clean up rendering resources if needed
    isInitialized = false;
    worldMap.reset();
    // Reset other renderable components
}