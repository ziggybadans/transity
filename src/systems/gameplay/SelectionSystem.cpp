#include "SelectionSystem.h"
#include "core/ServiceLocator.h"
#include "app/GameState.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include "Logger.h"
#include "imgui.h"
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

SelectionSystem::SelectionSystem(ServiceLocator& serviceLocator)
    : _serviceLocator(serviceLocator) {
    _mouseButtonConnection = _serviceLocator.eventBus.sink<MouseButtonPressedEvent>().connect<&SelectionSystem::onMouseButtonPressed>(this);
    LOG_DEBUG("SelectionSystem", "SelectionSystem created and connected to event bus.");
}

SelectionSystem::~SelectionSystem() {
    LOG_DEBUG("SelectionSystem", "SelectionSystem destroyed.");
}

void SelectionSystem::onMouseButtonPressed(const MouseButtonPressedEvent& event) {
    if (_serviceLocator.gameState.currentInteractionMode != InteractionMode::SELECT) {
        return;
    }

    if (event.button != sf::Mouse::Button::Left) {
        return;
    }

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    auto& registry = _serviceLocator.registry;

    // Clear the selection component from the previously selected entity
    if (_serviceLocator.gameState.selectedEntity.has_value()) {
        auto oldSelectedEntity = _serviceLocator.gameState.selectedEntity.value();
        if (registry.valid(oldSelectedEntity)) {
            registry.remove<SelectedComponent>(oldSelectedEntity);
        }
    }

    entt::entity clickedEntity = entt::null;

    // 1. Check for clickable components (stations, etc.)
    auto clickableView = registry.view<const PositionComponent, const ClickableComponent>();
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
        auto lineView = registry.view<const LineComponent>();
        float minDistanceSq = std::numeric_limits<float>::max();
        const float selectionThresholdSq = 10.0f * 10.0f; // 10px selection radius, squared

        for (auto entity : lineView) {
            const auto& line = lineView.get<const LineComponent>(entity);
            if (line.stops.size() < 2) continue;

            for (size_t i = 0; i < line.stops.size() - 1; ++i) {
                if (!registry.valid(line.stops[i]) || !registry.valid(line.stops[i + 1])) continue;

                const auto& pos1 = registry.get<const PositionComponent>(line.stops[i]).coordinates;
                const auto& pos2 = registry.get<const PositionComponent>(line.stops[i + 1]).coordinates;

                float distSq = distanceToSegmentSq(event.worldPosition, pos1, pos2);

                if (distSq < minDistanceSq && distSq < selectionThresholdSq) {
                    minDistanceSq = distSq;
                    clickedEntity = entity;
                }
            }
        }
    }

    if (clickedEntity != entt::null) {
        _serviceLocator.gameState.selectedEntity = clickedEntity;
        registry.emplace<SelectedComponent>(clickedEntity);
        LOG_INFO("SelectionSystem", "Entity %u selected.", entt::to_integral(clickedEntity));
    } else {
        if (_serviceLocator.gameState.selectedEntity.has_value()) {
            LOG_INFO("SelectionSystem", "Selection cleared.");
        }
        _serviceLocator.gameState.selectedEntity = std::nullopt;
    }
}