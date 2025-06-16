#include "SystemManager.h"
#include "../systems/CameraSystem.h"
#include "../systems/LineCreationSystem.h"
#include "../systems/StationPlacementSystem.h"
#include "../input/InputHandler.h"
#include "../graphics/UI.h"

// The constructor simply moves the provided unique_ptrs into its members
SystemManager::SystemManager(
    std::unique_ptr<CameraSystem> cameraSystem,
    std::unique_ptr<LineCreationSystem> lineCreationSystem,
    std::unique_ptr<StationPlacementSystem> stationPlacementSystem)
    : m_cameraSystem(std::move(cameraSystem)),
      m_lineCreationSystem(std::move(lineCreationSystem)),
      m_stationPlacementSystem(std::move(stationPlacementSystem)) {}

// The update method now calls the systems' simpler update methods
void SystemManager::update(sf::Time dt, InteractionMode mode) {
    m_cameraSystem->update();
    m_stationPlacementSystem->update(mode);
    // Note: LineCreationSystem is event-driven, so it's not called in the main update loop.
}

void SystemManager::processEvents(InputHandler& inputHandler, UI& ui) {
    // This method's dependencies are passed in because they are transient and specific to event processing
    m_lineCreationSystem->processEvents(inputHandler.getGameEvents(), ui.getUiEvents());
}

LineCreationSystem& SystemManager::getLineCreationSystem() {
    return *m_lineCreationSystem;
}
