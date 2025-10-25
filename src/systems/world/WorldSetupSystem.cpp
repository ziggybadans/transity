#include "WorldSetupSystem.h"
#include "Logger.h"
#include "WorldGenerationSystem.h"
#include "components/WorldComponents.h"
#include "app/LoadingState.h"
#include "render/Camera.h"
#include "render/Renderer.h"

#include <entt/entt.hpp>

WorldSetupSystem::WorldSetupSystem(entt::registry& registry, LoadingState& loadingState, WorldGenerationSystem& worldGenerationSystem, Renderer& renderer, Camera& camera) 
    : _registry(registry), _loadingState(loadingState), _worldGenerationSystem(worldGenerationSystem), _renderer(renderer), _camera(camera) {}

void WorldSetupSystem::init() {
    LOG_INFO("WorldSetupSystem", "Initializing world setup.");

    _loadingState.message = "Preparing world grid...";
    _loadingState.progress = 0.02f;

    auto worldGridEntity = _registry.create();
    _registry.emplace<WorldGridComponent>(worldGridEntity);
    LOG_DEBUG("WorldSetupSystem", "WorldGridComponent created with default values.");

    _loadingState.message = "World grid ready.";
    _loadingState.progress = 0.05f;

    _loadingState.message = "Configuring camera...";
    _loadingState.progress = 0.08f;
    sf::Vector2f worldSize = _worldGenerationSystem.getWorldSize();
    sf::Vector2f worldCenter = {worldSize.x / 2.0f, worldSize.y / 2.0f};

    auto &window = _renderer.getWindowInstance();

    float zoomFactor = 4.0f;
    sf::Vector2f initialViewSize = {worldSize.x / zoomFactor, worldSize.y / zoomFactor};
    _camera.setInitialView(window, worldCenter, initialViewSize);

    sf::Vector2u windowSize = window.getSize();
    _camera.onWindowResize(windowSize.x, windowSize.y);

    _loadingState.message = "Camera aligned.";
    _loadingState.progress = 0.1f;

    LOG_INFO("WorldSetupSystem", "World setup initialization completed.");
}

void WorldSetupSystem::update(sf::Time dt) {
    // This system does not need a per-frame update.
}
