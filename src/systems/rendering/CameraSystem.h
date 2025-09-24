#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"
#include "event/EventBus.h"
#include <entt/entt.hpp>

class Camera;
class Renderer;
class WorldGenerationSystem;
namespace sf {
class RenderWindow;
}

class CameraSystem : public ISystem, public IUpdatable {
public:
    explicit CameraSystem(Camera& camera, Renderer& renderer, WorldGenerationSystem& worldGenSystem, EventBus& eventBus);
    ~CameraSystem();

    void update(sf::Time dt) override;

private:
    void onCameraZoom(const CameraZoomEvent &event);
    void onCameraPan(const CameraPanEvent &event);

    Camera &m_camera;
    sf::RenderWindow &m_window;
    WorldGenerationSystem &m_worldGenSystem;

    entt::scoped_connection m_zoomConnection;
    entt::scoped_connection m_panConnection;
};