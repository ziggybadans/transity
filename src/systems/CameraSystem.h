#pragma once

#include "../core/SystemManager.h" // Include for ISystem
#include "../event/InputEvents.h"
#include <entt/entt.hpp>

// Forward declarations
class ServiceLocator;
class Camera;
namespace sf { class RenderWindow; }

class CameraSystem : public ISystem {
public:
    // Constructor now takes the ServiceLocator
    explicit CameraSystem(ServiceLocator& serviceLocator);
    ~CameraSystem();

private:
    // Event handler methods
    void onCameraZoom(const CameraZoomEvent& event);
    void onCameraPan(const CameraPanEvent& event);

    // Pointers to services obtained from the ServiceLocator
    Camera* m_camera;
    sf::RenderWindow* m_window;

    // Store connections to disconnect in the destructor
    entt::connection m_zoomConnection;
    entt::connection m_panConnection;
};
