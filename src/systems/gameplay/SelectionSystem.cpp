#include "SelectionSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"
#include "Logger.h"
#include "imgui.h"
#include "core/Pathfinder.h"
#include "event/UIEvents.h" // Add this include
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cmath>
#include <algorithm>
#include <limits>

namespace {
    // Calculates the squared distance from a point p to a line segment defined by endpoints v and w.
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
        // Logic for selecting a destination city
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
                        // Create passenger
                        auto passenger = _registry.create();
                        _registry.emplace<PassengerComponent>(passenger, origin, destination);
                        
                        // Add and populate PathComponent
                        auto& pathComponent = _registry.emplace<PathComponent>(passenger);
                        pathComponent.nodes = path;
                        pathComponent.currentNodeIndex = 0;

                        // Add passenger to origin city's waiting list
                        auto& originCity = _registry.get<CityComponent>(origin);
                        originCity.waitingPassengers.push_back(passenger);

                        LOG_INFO("SelectionSystem", "Passenger created from %u to %u with path size %zu", entt::to_integral(origin), entt::to_integral(destination), path.size());
                    } else {
                        LOG_WARN("SelectionSystem", "Could not find a path for passenger from %u to %u", entt::to_integral(origin), entt::to_integral(destination));
                    }

                    // Reset state regardless of pathfinding success
                    _gameState.passengerOriginStation = std::nullopt;
                    _eventBus.enqueue<InteractionModeChangeEvent>({InteractionMode::SELECT});
                }
                return; // Exit after handling click
            }
        }
    } else if (_gameState.currentInteractionMode != InteractionMode::SELECT) {
        return;
    }

    // Clear the selection component from ALL previously selected entities
    auto selectionView = _registry.view<SelectedComponent>();
    for (auto entity : selectionView) {
        _registry.remove<SelectedComponent>(entity);
    }

    entt::entity clickedEntity = entt::null;

    // 1. Check for clickable components (stations, etc.)
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

    // 2. If no clickable component was found, check for lines
    if (clickedEntity == entt::null) {
        auto lineView = _registry.view<const LineComponent>();
        float minDistanceSq = std::numeric_limits<float>::max();
        const float selectionThresholdSq = 10.0f * 10.0f; // 10px selection radius, squared

        for (auto entity : lineView) {
            const auto& line = lineView.get<const LineComponent>(entity);
            if (line.stops.size() < 2) continue;

            for (size_t i = 0; i < line.stops.size() - 1; ++i) {
                if (!_registry.valid(line.stops[i]) || !_registry.valid(line.stops[i + 1])) continue;

                const auto& pos1 = _registry.get<const PositionComponent>(line.stops[i]).coordinates;
                const auto& pos2 = _registry.get<const PositionComponent>(line.stops[i + 1]).coordinates;

                float distSq = distanceToSegmentSq(event.worldPosition, pos1, pos2);

                if (distSq < minDistanceSq && distSq < selectionThresholdSq) {
                    minDistanceSq = distSq;
                    clickedEntity = entity;
                }
            }
        }
    }

    if (clickedEntity != entt::null) {
        _gameState.selectedEntity = clickedEntity; // This line will be removed later
        _registry.emplace<SelectedComponent>(clickedEntity);
        _eventBus.enqueue<EntitySelectedEvent>({clickedEntity}); // Fire event
        LOG_DEBUG("SelectionSystem", "Entity %u selected.", entt::to_integral(clickedEntity));
    } else {
        if (_gameState.selectedEntity.has_value()) {
            LOG_DEBUG("SelectionSystem", "Selection cleared.");
        }
        _gameState.selectedEntity = std::nullopt; // This line will be removed later
        _eventBus.enqueue<EntityDeselectedEvent>({}); // Fire event
    }
}