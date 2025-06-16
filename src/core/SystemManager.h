#pragma once

#include <memory>
#include <SFML/System/Time.hpp>
#include "../input/InteractionMode.h"

// Forward declarations for systems and other dependencies
class CameraSystem;
class LineCreationSystem;
class StationPlacementSystem;
class InputHandler;
class UI;

class SystemManager {
public:
    // The constructor now takes ownership of the systems via unique_ptr
    SystemManager(
        std::unique_ptr<CameraSystem> cameraSystem,
        std::unique_ptr<LineCreationSystem> lineCreationSystem,
        std::unique_ptr<StationPlacementSystem> stationPlacementSystem
    );

    // The update signature is now much simpler
    void update(sf::Time dt, InteractionMode mode);
    void processEvents(InputHandler& inputHandler, UI& ui);

    // Getter remains the same
    LineCreationSystem& getLineCreationSystem();

private:
    std::unique_ptr<CameraSystem> m_cameraSystem;
    std::unique_ptr<LineCreationSystem> m_lineCreationSystem;
    std::unique_ptr<StationPlacementSystem> m_stationPlacementSystem;
};
