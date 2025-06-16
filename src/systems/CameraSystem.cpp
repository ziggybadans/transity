#include "CameraSystem.h"
#include "../Logger.h"

// Initialise references in the constructor's initialiser list
CameraSystem::CameraSystem(InputHandler& inputHandler, Camera& camera, sf::RenderWindow& window)
    : m_inputHandler(inputHandler), m_camera(camera), m_window(window) {}

void CameraSystem::update() {
    // Use the member variables instead of parameters
    const auto& commands = m_inputHandler.getCommands();
    for (const auto& command : commands) {
        switch (command.type) {
            case InputEventType::CAMERA_ZOOM:
                {
                    LOG_DEBUG("CameraSystem", "Processing CameraZoom command with delta: %.2f", command.data.zoomDelta);
                    sf::View& view = m_camera.getViewToModify();
                    sf::Vector2f worldPosBeforeZoom = m_window.mapPixelToCoords(command.data.mousePixelPosition, view);
                    m_camera.zoomView(command.data.zoomDelta);
                    sf::Vector2f worldPosAfterZoom = m_window.mapPixelToCoords(command.data.mousePixelPosition, view);
                    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
                    m_camera.moveView(offset);
                    LOG_TRACE("CameraSystem", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x, offset.y);
                }
                break;
            case InputEventType::CAMERA_PAN:
                LOG_DEBUG("CameraSystem", "Processing CameraPan command with direction: (%.1f, %.1f)", command.data.panDirection.x, command.data.panDirection.y);
                m_camera.moveView(command.data.panDirection);
                break;
            default:
                break;
        }
    }
}
