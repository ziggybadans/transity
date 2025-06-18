#include "SystemManager.h"
#include "../systems/CameraSystem.h"
#include "../systems/LineCreationSystem.h"
#include "../systems/StationPlacementSystem.h"

SystemManager::SystemManager(
    std::unique_ptr<CameraSystem> cameraSystem,
    std::unique_ptr<LineCreationSystem> lineCreationSystem,
    std::unique_ptr<StationPlacementSystem> stationPlacementSystem)
    : m_cameraSystem(std::move(cameraSystem)),
      m_lineCreationSystem(std::move(lineCreationSystem)),
      m_stationPlacementSystem(std::move(stationPlacementSystem)) {}

// The update method no longer needs to call individual system updates
// as they are now event-driven. This method could be removed entirely
// if no systems need a per-frame update. For now, we'll leave it empty.
void SystemManager::update(sf::Time dt) {
    // No systems currently require a per-frame update.
}

// The processEvents method is removed entirely.

LineCreationSystem& SystemManager::getLineCreationSystem() {
    return *m_lineCreationSystem;
}
