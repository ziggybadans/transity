#include "WorldSetupSystem.h"
#include "Logger.h"
#include "WorldGenerationSystem.h"
#include "components/WorldComponents.h"
#include "core/ServiceLocator.h"
#include "render/Camera.h"
#include "render/Renderer.h"

#include <entt/entt.hpp>

WorldSetupSystem::WorldSetupSystem(ServiceLocator &services) : m_services(services) {}

void WorldSetupSystem::init() {
    LOG_INFO("WorldSetupSystem", "Initializing world setup.");

    m_services.loadingState.message = "Creating world grid...";
    m_services.loadingState.progress = 0.1f;

    auto worldGridEntity = m_services.registry.create();
    m_services.registry.emplace<WorldGridComponent>(worldGridEntity);
    LOG_INFO("WorldSetupSystem", "WorldGridComponent created with default values.");

    m_services.loadingState.message = "Configuring camera...";
    auto &worldGenSystem = m_services.worldGenerationSystem;
    sf::Vector2f worldSize = worldGenSystem.getWorldSize();
    sf::Vector2f worldCenter = {worldSize.x / 2.0f, worldSize.y / 2.0f};

    auto &window = m_services.renderer.getWindowInstance();
    auto &camera = m_services.camera;

    float zoomFactor = 4.0f;
    sf::Vector2f initialViewSize = {worldSize.x / zoomFactor, worldSize.y / zoomFactor};
    camera.setInitialView(window, worldCenter, initialViewSize);

    sf::Vector2u windowSize = window.getSize();
    camera.onWindowResize(windowSize.x, windowSize.y);

    m_services.loadingState.progress = 0.2f;

    LOG_INFO("WorldSetupSystem", "World setup initialization completed.");
}

void WorldSetupSystem::update(sf::Time dt) {
    // This system does not need a per-frame update.
}
