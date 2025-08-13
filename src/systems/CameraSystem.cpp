#include "CameraSystem.h"
#include "../Logger.h"
#include "../core/Camera.h"
#include "../core/ServiceLocator.h"
#include "../graphics/Renderer.h"

#include <cassert>

CameraSystem::CameraSystem(ServiceLocator &serviceLocator)
    : m_camera(serviceLocator.camera), m_window(serviceLocator.renderer.getWindowInstance()) { // Change -> to .
    m_zoomConnection =
        serviceLocator.eventBus.sink<CameraZoomEvent>().connect<&CameraSystem::onCameraZoom>(this); // Change -> to .
    m_panConnection =
        serviceLocator.eventBus.sink<CameraPanEvent>().connect<&CameraSystem::onCameraPan>(this); // Change -> to .
    LOG_INFO("CameraSystem", "CameraSystem created and connected to EventBus.");
}

CameraSystem::~CameraSystem() {
    m_zoomConnection.release();
    m_panConnection.release();
    LOG_INFO("CameraSystem", "CameraSystem destroyed and disconnected from EventBus.");
}

void CameraSystem::onCameraZoom(const CameraZoomEvent &event) {
    assert(event.zoomDelta != 0.0f && "Camera zoom delta cannot be zero.");
    LOG_DEBUG("CameraSystem", "Processing CameraZoomEvent with delta: %.2f", event.zoomDelta);
    sf::View &view = m_camera.getViewToModify();
    sf::Vector2f worldPosBeforeZoom = m_window.mapPixelToCoords(event.mousePixelPosition, view);
    m_camera.zoomView(event.zoomDelta);
    sf::Vector2f worldPosAfterZoom = m_window.mapPixelToCoords(event.mousePixelPosition, view);
    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
    m_camera.moveView(offset);
    LOG_TRACE("CameraSystem", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x,
              offset.y);
}

void CameraSystem::onCameraPan(const CameraPanEvent &event) {
    LOG_DEBUG("CameraSystem", "Processing CameraPanEvent with direction: (%.1f, %.1f)",
              event.panDirection.x, event.panDirection.y);
    m_camera.moveView(event.panDirection);
}

void CameraSystem::update(sf::Time dt) {}