#include "CameraSystem.h"
#include "../Logger.h"

// The constructor now connects member functions to the event bus
CameraSystem::CameraSystem(EventBus& eventBus, Camera& camera, sf::RenderWindow& window)
    : m_camera(camera), m_window(window) {
    m_zoomConnection = eventBus.sink<CameraZoomEvent>().connect<&CameraSystem::onCameraZoom>(this);
    m_panConnection = eventBus.sink<CameraPanEvent>().connect<&CameraSystem::onCameraPan>(this);
    LOG_INFO("CameraSystem", "CameraSystem created and connected to EventBus.");
}

CameraSystem::~CameraSystem() {
    // It's good practice to disconnect from the event bus on destruction
    m_zoomConnection.release();
    m_panConnection.release();
    LOG_INFO("CameraSystem", "CameraSystem destroyed and disconnected from EventBus.");
}

// The logic from the old update() method is moved into these event handlers
void CameraSystem::onCameraZoom(const CameraZoomEvent& event) {
    LOG_DEBUG("CameraSystem", "Processing CameraZoomEvent with delta: %.2f", event.zoomDelta);
    sf::View& view = m_camera.getViewToModify();
    sf::Vector2f worldPosBeforeZoom = m_window.mapPixelToCoords(event.mousePixelPosition, view);
    m_camera.zoomView(event.zoomDelta);
    sf::Vector2f worldPosAfterZoom = m_window.mapPixelToCoords(event.mousePixelPosition, view);
    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
    m_camera.moveView(offset);
    LOG_TRACE("CameraSystem", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x, offset.y);
}

void CameraSystem::onCameraPan(const CameraPanEvent& event) {
    LOG_DEBUG("CameraSystem", "Processing CameraPanEvent with direction: (%.1f, %.1f)", event.panDirection.x, event.panDirection.y);
    m_camera.moveView(event.panDirection);
}
