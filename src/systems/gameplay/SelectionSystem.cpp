#include "SelectionSystem.h"
#include "core/ServiceLocator.h"
#include "app/GameState.h"
#include "components/GameLogicComponents.h"
#include "Logger.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>

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

    auto& registry = _serviceLocator.registry;
    auto view = registry.view<const PositionComponent, const ClickableComponent>();
    
    entt::entity clickedEntity = entt::null;

    for (auto entity : view) {
        const auto& position = view.get<const PositionComponent>(entity);
        const auto& clickable = view.get<const ClickableComponent>(entity);

        sf::Vector2f diff = position.coordinates - event.worldPosition;
        float distanceSq = diff.x * diff.x + diff.y * diff.y;
        float radiusSq = clickable.boundingRadius.value * clickable.boundingRadius.value;

        if (distanceSq <= radiusSq) {
            clickedEntity = entity;
            break; // Found the top-most entity
        }
    }

    if (clickedEntity != entt::null) {
        _serviceLocator.gameState.selectedEntity = clickedEntity;
        LOG_INFO("SelectionSystem", "Entity %u selected.", entt::to_integral(clickedEntity));
    } else {
        if (_serviceLocator.gameState.selectedEntity.has_value()) {
            LOG_INFO("SelectionSystem", "Selection cleared.");
        }
        _serviceLocator.gameState.selectedEntity = std::nullopt;
    }
}