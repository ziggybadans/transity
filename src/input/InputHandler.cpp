#include "InputHandler.h"
#include "Constants.h"
#include "Logger.h"
#include "imgui.h"

InputHandler::InputHandler(EventBus &eventBus, Camera &camera)
    : _eventBus(eventBus), _camera(camera), _zoomFactor(Constants::ZOOM_FACTOR),
      _unzoomFactor(Constants::UNZOOM_FACTOR) {
    LOG_DEBUG("Input", "InputHandler created.");
}

void InputHandler::handleGameEvent(const sf::Event &event, sf::RenderWindow &window) {
    if (event.is<sf::Event::Closed>()) {
        LOG_INFO("Input", "Window close event received.");
        _eventBus.enqueue<WindowCloseEvent>();
        return;
    }

    if (!_isWindowFocused) {
        return;
    }

    const ImGuiIO &io = ImGui::GetIO();
    const bool uiCapturesMouse = io.WantCaptureMouse;
    const bool uiCapturesKeyboard = io.WantCaptureKeyboard;

    if (auto *scrollData = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (uiCapturesMouse) {
            return;
        }
        handleMouseScroll(*scrollData, window);
    } else if (auto *pressData = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (uiCapturesMouse) {
            return;
        }
        handleMouseButtonPress(*pressData, window);
    } else if (auto *releaseData = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (uiCapturesMouse) {
            return;
        }
        handleMouseButtonRelease(*releaseData, window);
    } else if (auto *moveData = event.getIf<sf::Event::MouseMoved>()) {
        if (uiCapturesMouse) {
            return;
        }
        handleMouseMove(*moveData, window);
    } else if (auto *keyData = event.getIf<sf::Event::KeyPressed>()) {
        if (uiCapturesKeyboard) {
            return;
        }
        handleKeyPress(*keyData);
    }
}

void InputHandler::handleMouseScroll(const sf::Event::MouseWheelScrolled &scrollData,
                                     sf::RenderWindow &window) {
    if (scrollData.wheel == sf::Mouse::Wheel::Vertical) {
        LOG_TRACE("Input", "Mouse wheel scrolled: delta %.1f", scrollData.delta);
        float zoomDelta = 0.0f;
        if (scrollData.delta > 0) {
            zoomDelta = _zoomFactor;
            LOG_TRACE("Input", "Zoom in event generated.");
        } else if (scrollData.delta < 0) {
            zoomDelta = _unzoomFactor;
            LOG_TRACE("Input", "Zoom out event generated.");
        }
        if (zoomDelta != 0.0f) {
            _eventBus.enqueue<CameraZoomEvent>({zoomDelta, sf::Mouse::getPosition(window)});
        }
    }
}

void InputHandler::handleMouseButtonPress(const sf::Event::MouseButtonPressed &pressData,
                                          sf::RenderWindow &window) {
    sf::Vector2i pixelPos = pressData.position;
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, _camera.getView());

    _eventBus.enqueue<MouseButtonPressedEvent>({pressData.button, pixelPos, worldPos});

    LOG_DEBUG("Input", "MouseButtonPressedEvent generated for button %d at world (%.1f, %.1f)",
              pressData.button, worldPos.x, worldPos.y);
}

void InputHandler::handleMouseButtonRelease(const sf::Event::MouseButtonReleased &releaseData,
                                            sf::RenderWindow &window) {
    sf::Vector2i pixelPos = releaseData.position;
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, _camera.getView());

    _eventBus.enqueue<MouseButtonReleasedEvent>({releaseData.button, pixelPos, worldPos});

    LOG_DEBUG("Input", "MouseButtonReleasedEvent generated for button %d at world (%.1f, %.1f)",
              releaseData.button, worldPos.x, worldPos.y);
}

void InputHandler::handleMouseMove(const sf::Event::MouseMoved &moveData,
                                   sf::RenderWindow &window) {
    sf::Vector2i pixelPos = moveData.position;
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, _camera.getView());

    _eventBus.enqueue<MouseMovedEvent>({pixelPos, worldPos});
}

void InputHandler::handleKeyPress(const sf::Event::KeyPressed &keyData) {
    _eventBus.enqueue<KeyPressedEvent>({keyData.code});
}

void InputHandler::setWindowFocus(bool isFocused) noexcept {
    _isWindowFocused = isFocused;
}

void InputHandler::update(sf::Time dt) {
    if (!_isWindowFocused) {
        return;
    }

    const ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

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
        const sf::View &view = _camera.getView();
        sf::Vector2f viewSize = view.getSize();
        float dynamicCameraSpeed = viewSize.y * Constants::DYNAMIC_CAMERA_SPEED_MULTIPLIER;
        sf::Vector2f panVector = panDirection * dynamicCameraSpeed * dt.asSeconds();
        _eventBus.enqueue<CameraPanEvent>({panVector});
        LOG_TRACE("Input", "CameraPan event generated with direction (%.1f, %.1f).", panVector.x,
                  panVector.y);
    }
}
