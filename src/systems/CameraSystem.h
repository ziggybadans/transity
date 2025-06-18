#pragma once

#include "../core/Camera.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include "../event/EventBus.h"
#include "../event/InputEvents.h"

class CameraSystem {
public:
    // The constructor now takes the EventBus and connects to it.
    CameraSystem(EventBus& eventBus, Camera& camera, sf::RenderWindow& window);
    ~CameraSystem();

    // The update method is no longer needed as the system is purely event-driven.

private:
    // Event handler methods
    void onCameraZoom(const CameraZoomEvent& event);
    void onCameraPan(const CameraPanEvent& event);

    Camera& m_camera;
    sf::RenderWindow& m_window;

    // Store connections to disconnect in the destructor
    entt::connection m_zoomConnection;
    entt::connection m_panConnection;
};
