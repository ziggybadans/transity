#include "InputHandler.h"
#include "../Logger.h"
#include "../core/Components.h"
#include "../core/Constants.h"
#include <iostream>
#include <string>
#include <vector>

// Constructor now takes and stores the EventBus reference
InputHandler::InputHandler(ServiceLocator& serviceLocator)
    : _services(serviceLocator),
      _zoomFactor(Constants::ZOOM_FACTOR),
      _unzoomFactor(Constants::UNZOOM_FACTOR) {
    LOG_INFO("Input", "InputHandler created.");
}

void InputHandler::handleGameEvent(const sf::Event& event, sf::RenderWindow& window) {
    EventBus* eventBus = _services.eventBus;
    GameState* gameState = _services.gameState;
    Camera* camera = _services.camera;
    entt::registry* registry = _services.registry;

    if (!eventBus || !gameState || !camera || !registry) {
        LOG_ERROR("Input", "Service locator is missing required services for InputHandler.");
        return;
    }

    if (event.is<sf::Event::Closed>()) {
        LOG_INFO("Input", "Window close event received.");
        eventBus->trigger<WindowCloseEvent>();
    } else if (auto* scrollData = event.getIf<sf::Event::MouseWheelScrolled>()) {
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
                 eventBus->trigger<CameraZoomEvent>({zoomDelta, sf::Mouse::getPosition(window)});
            }
        }
    } else if (auto* pressData = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressData->button == sf::Mouse::Button::Right) {
            if (gameState->currentInteractionMode == InteractionMode::CREATE_STATION) {
                sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePixelPos, camera->getView());
                LOG_DEBUG("Input", "Right mouse button pressed at screen ( %d, %d ), world (%.1f, %.1f). TryPlaceStation event generated.", mousePixelPos.x, mousePixelPos.y, worldPos.x, worldPos.y);
                eventBus->trigger<TryPlaceStationEvent>({worldPos});
            }
        } else if (pressData->button == sf::Mouse::Button::Left) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera->getView());

            if (gameState->currentInteractionMode == InteractionMode::CREATE_LINE) {
                LOG_DEBUG("Input", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f).", worldPos.x, worldPos.y);
                auto view = registry->view<PositionComponent, ClickableComponent>();
                bool stationClickedThisPress = false;
                for (auto entity_id : view) {
                    const auto& pos = view.get<PositionComponent>(entity_id);
                    const auto& clickable = view.get<ClickableComponent>(entity_id);

                    sf::Vector2f diff = worldPos - pos.coordinates;
                    float distanceSquared = (diff.x * diff.x) + (diff.y * diff.y);

                    if (distanceSquared <= clickable.boundingRadius * clickable.boundingRadius) {
                        LOG_DEBUG("Input", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f).", worldPos.x, worldPos.y);
                        eventBus->trigger<AddStationToLineEvent>({entity_id});
                        LOG_DEBUG("Input", "AddStationToLineEvent created for entity %u.", static_cast<unsigned int>(entity_id));
                        stationClickedThisPress = true;
                        break;
                    }
                }
                if (!stationClickedThisPress) {
                    LOG_TRACE("Input", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f) but no station found.", worldPos.x, worldPos.y);
                }
            }
        }
    }
}

void InputHandler::update(sf::Time dt) {
    EventBus* eventBus = _services.eventBus;
    Camera* camera = _services.camera;

    if (!eventBus || !camera) {
        LOG_ERROR("Input", "Service locator is missing required services for InputHandler update.");
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
        const sf::View& view = camera->getView();
        sf::Vector2f viewSize = view.getSize();
        float dynamicCameraSpeed = viewSize.y * Constants::DYNAMIC_CAMERA_SPEED_MULTIPLIER;
        sf::Vector2f panVector = panDirection * dynamicCameraSpeed * dt.asSeconds();
        eventBus->trigger<CameraPanEvent>({panVector});
        LOG_TRACE("Input", "CameraPan event generated with direction (%.1f, %.1f).", panVector.x, panVector.y);
    }
}
