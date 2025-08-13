#include "InputHandler.h"
#include "../Logger.h"
#include "../core/Constants.h"
#include <iostream>
#include <string>
#include <vector>

InputHandler::InputHandler(ServiceLocator &serviceLocator)
    : _services(serviceLocator), _zoomFactor(Constants::ZOOM_FACTOR),
      _unzoomFactor(Constants::UNZOOM_FACTOR) {
    LOG_INFO("Input", "InputHandler created.");
}

void InputHandler::handleGameEvent(const sf::Event &event, sf::RenderWindow &window) {
    if (event.is<sf::Event::Closed>()) {
        LOG_INFO("Input", "Window close event received.");
        _services.eventBus.enqueue<WindowCloseEvent>();
    } else if (auto *scrollData = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (scrollData->wheel == sf::Mouse::Wheel::Vertical) {
            LOG_DEBUG("Input", "Mouse wheel scrolled: delta %.1f", scrollData->delta);
            float zoomDelta = 0.0f;
            if (scrollData->delta > 0) {
                zoomDelta = _zoomFactor;
                LOG_TRACE("Input", "Zoom in event generated.");
            } else if (scrollData->delta < 0) {
                zoomDelta = _unzoomFactor;
                LOG_TRACE("Input", "Zoom out event generated.");
            }
            if (zoomDelta != 0.0f) {
                _services.eventBus.enqueue<CameraZoomEvent>(
                    {zoomDelta, sf::Mouse::getPosition(window)});
            }
        }
    } else if (auto *pressData = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2i pixelPos = pressData->position;
        sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, _services.camera.getView());

        _services.eventBus.enqueue<MouseButtonPressedEvent>(
            {pressData->button, pixelPos, worldPos});

        LOG_DEBUG("Input", "MouseButtonPressedEvent generated for button %d at world (%.1f, %.1f)",
                  pressData->button, worldPos.x, worldPos.y);
    }
}

void InputHandler::update(sf::Time dt) {
    sf::Vector2f panDirection(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        panDirection.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        panDirection.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        panDirection.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        panDirection.x += 1.0f;
    }

    if (panDirection.x != 0.f || panDirection.y != 0.f) {
        const sf::View &view = _services.camera.getView();
        sf::Vector2f viewSize = view.getSize();
        float dynamicCameraSpeed = viewSize.y * Constants::DYNAMIC_CAMERA_SPEED_MULTIPLIER;
        sf::Vector2f panVector = panDirection * dynamicCameraSpeed * dt.asSeconds();
        _services.eventBus.enqueue<CameraPanEvent>({panVector});
        LOG_TRACE("Input", "CameraPan event generated with direction (%.1f, %.1f).", panVector.x,
                  panVector.y);
    }
}
