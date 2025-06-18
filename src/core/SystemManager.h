#pragma once

#include <memory>
#include <SFML/System/Time.hpp>
#include "../input/InteractionMode.h"

// Forward declarations
class CameraSystem;
class LineCreationSystem;
class StationPlacementSystem;

class SystemManager {
public:
    SystemManager(
        std::unique_ptr<CameraSystem> cameraSystem,
        std::unique_ptr<LineCreationSystem> lineCreationSystem,
        std::unique_ptr<StationPlacementSystem> stationPlacementSystem
    );

    // Update method is now simpler, only taking dt
    void update(sf::Time dt);
    
    // processEvents is no longer needed
    // void processEvents(InputHandler& inputHandler, UI& ui);

    LineCreationSystem& getLineCreationSystem();

private:
    std::unique_ptr<CameraSystem> m_cameraSystem;
    std::unique_ptr<LineCreationSystem> m_lineCreationSystem;
    std::unique_ptr<StationPlacementSystem> m_stationPlacementSystem;
};
