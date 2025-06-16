#pragma once

#include <memory>
#include <entt/entt.hpp>
#include <SFML/System/Time.hpp>
#include "../systems/CameraSystem.h"
#include "../systems/LineCreationSystem.h"
#include "../systems/StationPlacementSystem.h"
#include "../graphics/ColorManager.h"
#include "EntityFactory.h"

// Forward declarations
class InputHandler;
class UI;
class Renderer;
class Camera;

class SystemManager {
public:
    SystemManager(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager);

    // Update the signature to accept the registry and entity factory
    void update(sf::Time dt, InputHandler& inputHandler, UI& ui, Camera& camera, Renderer& renderer, entt::registry& registry, EntityFactory& entityFactory);
    void processEvents(InputHandler& inputHandler, UI& ui);

    LineCreationSystem& getLineCreationSystem() { return *_lineCreationSystem; }

private:
    std::unique_ptr<CameraSystem> _cameraSystem;
    std::unique_ptr<LineCreationSystem> _lineCreationSystem;
    std::unique_ptr<StationPlacementSystem> _stationPlacementSystem;
};
