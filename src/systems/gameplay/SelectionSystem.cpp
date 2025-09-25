#include "SelectionSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"
#include "Logger.h"
#include "imgui.h"
#include "core/Pathfinder.h"
#include "event/UIEvents.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cmath>
#include <algorithm>
#include <limits>

namespace {
    float distanceToSegmentSq(sf::Vector2f p, sf::Vector2f v, sf::Vector2f w) {
        float l2 = (w.x - v.x) * (w.x - v.x) + (w.y - v.y) * (w.y - v.y);
        if (l2 == 0.0) {
            return (p.x - v.x) * (p.x - v.x) + (p.y - v.y) * (p.y - v.y);
        }
        float t = std::max(0.f, std::min(1.f, ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2));
        sf::Vector2f projection = v + t * (w - v);
        sf::Vector2f d = p - projection;
        return d.x * d.x + d.y * d.y;
    }
}

SelectionSystem::SelectionSystem(entt::registry& registry, EventBus& eventBus, GameState& gameState, Pathfinder& pathfinder)
    : _registry(registry),
      _eventBus(eventBus),
      _gameState(gameState),
      _pathfinder(pathfinder) {
    _mouseButtonConnection = _eventBus.sink<MouseButtonPressedEvent>().connect<&SelectionSystem::onMouseButtonPressed>(this);
    LOG_DEBUG("SelectionSystem", "SelectionSystem created and connected to event bus.");
}

SelectionSystem::~SelectionSystem() {
    LOG_DEBUG("SelectionSystem", "SelectionSystem destroyed.");
}

void SelectionSystem::onMouseButtonPressed(const MouseButtonPressedEvent& event) {
    if (event.button != sf::Mouse::Button::Left) {
        return;
    }

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (_gameState.currentInteractionMode == InteractionMode::CREATE_PASSENGER) {
        auto clickableView = _registry.view<const PositionComponent, const ClickableComponent, const CityComponent>();
        for (auto entity : clickableView) {
            const auto& position = clickableView.get<const PositionComponent>(entity);
            const auto& clickable = clickableView.get<const ClickableComponent>(entity);

            sf::Vector2f diff = position.coordinates - event.worldPosition;
            float distanceSq = diff.x * diff.x + diff.y * diff.y;
            float radiusSq = clickable.boundingRadius.value * clickable.boundingRadius.value;

            if (distanceSq <= radiusSq) {
                if (_gameState.passengerOriginStation.has_value() && _gameState.passengerOriginStation.value() != entity) {
                    entt::entity origin = _gameState.passengerOriginStation.value();
                    entt::entity destination = entity;

                    std::vector<entt::entity> path = _pathfinder.findPath(origin, destination);

                    if (!path.empty()) {
                        auto passenger = _registry.create();
                        _registry.emplace<PassengerComponent>(passenger, origin, destination);
                        
                        auto& pathComponent = _registry.emplace<PathComponent>(passenger);
                        pathComponent.nodes = path;
                        pathComponent.currentNodeIndex = 0;

                        auto& originCity = _registry.get<CityComponent>(origin);
                        originCity.waitingPassengers.push_back(passenger);

                        LOG_INFO("SelectionSystem", "Passenger created from %u to %u with path size %zu", entt::to_integral(origin), entt::to_integral(destination), path.size());
                    } else {
                        LOG_WARN("SelectionSystem", "Could not find a path for passenger from %u to %u", entt::to_integral(origin), entt::to_integral(destination));
                    }

                    _gameState.passengerOriginStation = std::nullopt;
                    _eventBus.enqueue<InteractionModeChangeEvent>({InteractionMode::SELECT});
                }
                return;
            }
        }
    } else if (_gameState.currentInteractionMode != InteractionMode::SELECT) {
        return;
    }

    auto selectionView = _registry.view<SelectedComponent>();
    for (auto entity : selectionView) {
        _registry.remove<SelectedComponent>(entity);
    }

    entt::entity clickedEntity = entt::null;

    auto clickableView = _registry.view<const PositionComponent, const ClickableComponent>();
    for (auto entity : clickableView) {
        const auto& position = clickableView.get<const PositionComponent>(entity);
        const auto& clickable = clickableView.get<const ClickableComponent>(entity);

        sf::Vector2f diff = position.coordinates - event.worldPosition;
        float distanceSq = diff.x * diff.x + diff.y * diff.y;
        float radiusSq = clickable.boundingRadius.value * clickable.boundingRadius.value;

        if (distanceSq <= radiusSq) {
            clickedEntity = entity;
            break; 
        }
    }

    if (clickedEntity == entt::null) {
        auto lineView = _registry.view<const LineComponent>();
        float minDistanceSq = std::numeric_limits<float>::max();
        const float selectionThresholdSq = 10.0f * 10.0f;

        for (auto entity : lineView) {
            const auto& line = lineView.get<const LineComponent>(entity);
            if (line.points.size() < 2) continue;

            for (size_t i = 0; i < line.points.size() - 1; ++i) {
                const auto& pos1 = line.points[i].position;
                const auto& pos2 = line.points[i + 1].position;

                float distSq = distanceToSegmentSq(event.worldPosition, pos1, pos2);

                if (distSq < minDistanceSq && distSq < selectionThresholdSq) {
                    minDistanceSq = distSq;
                    clickedEntity = entity;
                }
            }
        }
    }

    if (clickedEntity != entt::null) {
        _gameState.selectedEntity = clickedEntity;
        _registry.emplace<SelectedComponent>(clickedEntity);
        _eventBus.enqueue<EntitySelectedEvent>({clickedEntity});
        LOG_DEBUG("SelectionSystem", "Entity %u selected.", entt::to_integral(clickedEntity));
    } else {
        if (_gameState.selectedEntity.has_value()) {
            LOG_DEBUG("SelectionSystem", "Selection cleared.");
        }
        _gameState.selectedEntity = std::nullopt;
        _eventBus.enqueue<EntityDeselectedEvent>({});
    }
}