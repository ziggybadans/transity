#include "InputHandler.h"
#include "Logger.h"
#include "Components.h"
#include <iostream>
#include <string>
#include <vector>
#include <variant>

InputHandler::InputHandler()
    : cameraSpeed(200.0f)
    , zoomFactor(0.9f)
    , unzoomFactor(1.0f / zoomFactor) {
    LOG_INFO("Input", "InputHandler created.");
}

void InputHandler::handleGameEvent(const sf::Event& event, InteractionMode currentMode, Camera& camera, sf::RenderWindow& window, entt::registry& registry) {
    if (event.is<sf::Event::Closed>()) {
        LOG_INFO("Input", "Window close event received.");
        m_commands.push_back({InputEventType::WindowClose, {}});
    } else if (auto* scrollData = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (scrollData->wheel == sf::Mouse::Wheel::Vertical) {
            LOG_DEBUG("Input", "Mouse wheel scrolled: delta %.1f", scrollData->delta);
            InputData data;
            data.mousePixelPosition = sf::Mouse::getPosition(window);
            if (scrollData->delta > 0) {
                data.zoomDelta = zoomFactor;
                LOG_TRACE("Input", "Zoom in command generated.");
            } else if (scrollData->delta < 0) {
                data.zoomDelta = unzoomFactor;
                LOG_TRACE("Input", "Zoom out command generated.");
            }
            if (data.zoomDelta != 0.0f) {
                 m_commands.push_back({InputEventType::CameraZoom, data});
            }
        }
    } else if (auto* pressData = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressData->button == sf::Mouse::Button::Right) {
            if (currentMode == InteractionMode::CREATE_STATION) {
                sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePixelPos, camera.getView());
                LOG_DEBUG("Input", "Right mouse button pressed at screen ( %d, %d ), world (%.1f, %.1f). TryPlaceStation command generated.", mousePixelPos.x, mousePixelPos.y, worldPos.x, worldPos.y);
                InputData data;
                data.worldPosition = worldPos;
                m_commands.push_back({InputEventType::TryPlaceStation, data});
            }
        } else if (pressData->button == sf::Mouse::Button::Left) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera.getView());

            if (currentMode == InteractionMode::CREATE_LINE) {
                LOG_DEBUG("Input", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f).", worldPos.x, worldPos.y);
                auto view = registry.view<PositionComponent, ClickableComponent>();
                bool stationClickedThisPress = false;
                for (auto entity_id : view) {
                    const auto& pos = view.get<PositionComponent>(entity_id);
                    const auto& clickable = view.get<ClickableComponent>(entity_id);

                    sf::Vector2f diff = worldPos - pos.coordinates;
                    float distanceSquared = (diff.x * diff.x) + (diff.y * diff.y);

                    if (distanceSquared <= clickable.boundingRadius * clickable.boundingRadius) {
                        LOG_DEBUG("Input", "Mouse click in CREATE_LINE mode at world (%.1f, %.1f).", worldPos.x, worldPos.y);
                        AddStationToLineEvent event;
                        event.stationEntity = entity_id;
                        m_gameEvents.emplace_back(event);
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
    sf::Vector2f panDirection(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        panDirection.y -= 1.0f;
        LOG_TRACE("Input", "Key W pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        panDirection.y += 1.0f;
        LOG_TRACE("Input", "Key S pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        panDirection.x -= 1.0f;
        LOG_TRACE("Input", "Key A pressed.");
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        panDirection.x += 1.0f;
        LOG_TRACE("Input", "Key D pressed.");
    }

    if (panDirection.x != 0.f || panDirection.y != 0.f) {
        InputData data;
        data.panDirection = panDirection * cameraSpeed * dt.asSeconds();
        m_commands.push_back({InputEventType::CameraPan, data});
        LOG_TRACE("Input", "CameraPan command generated with direction (%.1f, %.1f).", data.panDirection.x, data.panDirection.y);
    }
}

const std::vector<InputCommand>& InputHandler::getCommands() const {
    return m_commands;
}

void InputHandler::clearCommands() {
    m_commands.clear();
    LOG_TRACE("Input", "Input commands cleared.");
}

void InputHandler::addCommand(const InputCommand& command) {
    m_commands.push_back(command);
    LOG_TRACE("Input", "Input command added: %d", static_cast<int>(command.type));
}

const std::vector<std::variant<AddStationToLineEvent, FinalizeLineEvent>>& InputHandler::getGameEvents() const {
    return m_gameEvents;
}

void InputHandler::clearGameEvents() {
    m_gameEvents.clear();
    LOG_TRACE("Input", "Game events cleared.");
}