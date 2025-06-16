#include "SystemManager.h"
#include "../input/InputHandler.h"
#include "../graphics/UI.h"
#include "../graphics/Renderer.h"
#include "../core/Camera.h"

SystemManager::SystemManager(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager) {
    _cameraSystem = std::make_unique<CameraSystem>();
    _lineCreationSystem = std::make_unique<LineCreationSystem>(registry, entityFactory, colorManager);
    _stationPlacementSystem = std::make_unique<StationPlacementSystem>();
}

// The implementation now uses the passed-in registry and factory
void SystemManager::update(sf::Time dt, InputHandler& inputHandler, UI& ui, Camera& camera, Renderer& renderer, entt::registry& registry, EntityFactory& entityFactory) {
    _cameraSystem->update(inputHandler, camera, renderer.getWindowInstance());
    // Correctly use the dependencies passed from Game
    _stationPlacementSystem->update(inputHandler, ui, registry, entityFactory);
}

void SystemManager::processEvents(InputHandler& inputHandler, UI& ui) {
    _lineCreationSystem->processEvents(inputHandler.getGameEvents(), ui.getUiEvents());
}
