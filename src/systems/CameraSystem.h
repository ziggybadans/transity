#pragma once

#include "../core/ISystem.h"
#include "../core/SystemManager.h"  
#include "../event/InputEvents.h"
#include <entt/entt.hpp>


class ServiceLocator;
class Camera;
namespace sf {
class RenderWindow;
}

class CameraSystem : public ISystem {
public:
    
    explicit CameraSystem(ServiceLocator &serviceLocator);
    ~CameraSystem();

    void update(sf::Time dt) override;

private:
    
    void onCameraZoom(const CameraZoomEvent &event);
    void onCameraPan(const CameraPanEvent &event);

    
    Camera *m_camera;
    sf::RenderWindow *m_window;

    
    entt::connection m_zoomConnection;
    entt::connection m_panConnection;
};
