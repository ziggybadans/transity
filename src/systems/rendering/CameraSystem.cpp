#include "CameraSystem.h"
#include "Constants.h"
#include "Logger.h"
#include "core/ServiceLocator.h"
#include "render/Camera.h"
#include "render/Renderer.h"

#include <algorithm>

#include <cassert>

CameraSystem::CameraSystem(ServiceLocator &serviceLocator)
    : m_camera(serviceLocator.camera), m_window(serviceLocator.renderer.getWindowInstance()),
      m_worldGenSystem(serviceLocator.worldGenerationSystem) {
    m_zoomConnection =
        serviceLocator.eventBus.sink<CameraZoomEvent>().connect<&CameraSystem::onCameraZoom>(
            this);  // Change -> to .
    m_panConnection =
        serviceLocator.eventBus.sink<CameraPanEvent>().connect<&CameraSystem::onCameraPan>(
            this);  // Change -> to .
   LOG_DEBUG("CameraSystem", "CameraSystem created and connected to EventBus.");
}

CameraSystem::~CameraSystem() {
    m_zoomConnection.release();
    m_panConnection.release();
    LOG_DEBUG("CameraSystem", "CameraSystem destroyed and disconnected from EventBus.");
}

void CameraSystem::onCameraZoom(const CameraZoomEvent &event) {
    assert(event.zoomDelta != 0.0f && "Camera zoom delta cannot be zero.");
    LOG_TRACE("CameraSystem", "Processing CameraZoomEvent with delta: %.2f", event.zoomDelta);
    sf::View &view = m_camera.getViewToModify();

    const float currentZoom = m_camera.getZoom();
    float factor = event.zoomDelta;

    // Desired zoom after applying factor: zoom' = currentZoom * (1/factor)
    float newZoom = currentZoom * (1.0f / factor);
    const float minZoom = Constants::CAMERA_MIN_ZOOM;  // most zoomed out
    const float maxZoom = Constants::CAMERA_MAX_ZOOM;  // most zoomed in

    if (newZoom > maxZoom) {
        // Adjust factor to hit exactly maxZoom: maxZoom = currentZoom * (1/f)
        factor = currentZoom / maxZoom;
    } else if (newZoom < minZoom) {
        factor = currentZoom / minZoom;
    }

    sf::Vector2f worldPosBeforeZoom = m_window.mapPixelToCoords(event.mousePixelPosition, view);
    m_camera.zoomView(factor);
    sf::Vector2f worldPosAfterZoom = m_window.mapPixelToCoords(event.mousePixelPosition, view);
    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
    m_camera.moveView(offset);
    LOG_TRACE("CameraSystem", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x,
              offset.y);

    // Clamp center within world bounds after zoom
    const sf::Vector2f worldSize = m_worldGenSystem.getWorldSize();
    sf::Vector2f center = view.getCenter();
    const sf::Vector2f size = view.getSize();

    if (worldSize.x > 0.f && worldSize.y > 0.f) {
        const float halfW = size.x * 0.5f;
        const float halfH = size.y * 0.5f;
        float minX = halfW;
        float maxX = std::max(halfW, worldSize.x - halfW);
        float minY = halfH;
        float maxY = std::max(halfH, worldSize.y - halfH);

        // If view is larger than world, center it
        if (size.x >= worldSize.x) {
            center.x = worldSize.x * 0.5f;
        } else {
            center.x = std::clamp(center.x, minX, maxX);
        }
        if (size.y >= worldSize.y) {
            center.y = worldSize.y * 0.5f;
        } else {
            center.y = std::clamp(center.y, minY, maxY);
        }
        view.setCenter(center);
    }
}

void CameraSystem::onCameraPan(const CameraPanEvent &event) {
    LOG_TRACE("CameraSystem", "Processing CameraPanEvent with direction: (%.1f, %.1f)",
              event.panDirection.x, event.panDirection.y);
    sf::View &view = m_camera.getViewToModify();
    const sf::Vector2f worldSize = m_worldGenSystem.getWorldSize();

    if (worldSize.x <= 0.f || worldSize.y <= 0.f) {
        // Fallback if world not initialized
        m_camera.moveView(event.panDirection);
        return;
    }

    sf::Vector2f center = view.getCenter() + event.panDirection;
    const sf::Vector2f size = view.getSize();
    const float halfW = size.x * 0.5f;
    const float halfH = size.y * 0.5f;

    float minX = halfW;
    float maxX = std::max(halfW, worldSize.x - halfW);
    float minY = halfH;
    float maxY = std::max(halfH, worldSize.y - halfH);

    if (size.x >= worldSize.x) {
        center.x = worldSize.x * 0.5f;
    } else {
        center.x = std::clamp(center.x, minX, maxX);
    }
    if (size.y >= worldSize.y) {
        center.y = worldSize.y * 0.5f;
    } else {
        center.y = std::clamp(center.y, minY, maxY);
    }

    view.setCenter(center);
}

void CameraSystem::update(sf::Time dt) {}
