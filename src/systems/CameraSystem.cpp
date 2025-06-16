#include "CameraSystem.h"
#include "../Logger.h"
#include "../core/Camera.h"
#include "../input/InputHandler.h"
#include <SFML/Graphics/RenderWindow.hpp>

void CameraSystem::update(InputHandler& inputHandler, Camera& camera, sf::RenderWindow& window) {
    const auto& commands = inputHandler.getCommands();
    for (const auto& command : commands) {
        switch (command.type) {
            case InputEventType::CAMERA_ZOOM:
                {
                    LOG_DEBUG("CameraSystem", "Processing CameraZoom command with delta: %.2f", command.data.zoomDelta);
                    sf::View& view = camera.getViewToModify();
                    sf::Vector2f worldPosBeforeZoom = window.mapPixelToCoords(command.data.mousePixelPosition, view);
                    camera.zoomView(command.data.zoomDelta);
                    sf::Vector2f worldPosAfterZoom = window.mapPixelToCoords(command.data.mousePixelPosition, view);
                    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
                    camera.moveView(offset);
                    LOG_TRACE("CameraSystem", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x, offset.y);
                }
                break;
            case InputEventType::CAMERA_PAN:
                LOG_DEBUG("CameraSystem", "Processing CameraPan command with direction: (%.1f, %.1f)", command.data.panDirection.x, command.data.panDirection.y);
                camera.moveView(command.data.panDirection);
                break;
            default:
                break;
        }
    }
}