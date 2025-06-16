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

// Pass the mode down to the placement system
void SystemManager::update(sf::Time dt, InputHandler& inputHandler, InteractionMode mode, Camera& camera, Renderer& renderer, entt::registry& registry, EntityFactory& entityFactory) {
    _cameraSystem->update(inputHandler, camera, renderer.getWindowInstance());
    _stationPlacementSystem->update(inputHandler, mode, registry, entityFactory);
}

void SystemManager::processEvents(InputHandler& inputHandler, UI& ui) {
    _lineCreationSystem->processEvents(inputHandler.getGameEvents(), ui.getUiEvents());
}
