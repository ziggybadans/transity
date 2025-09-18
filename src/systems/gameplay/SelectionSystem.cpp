#include "SelectionSystem.h"
#include "core/ServiceLocator.h"
#include "app/GameState.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
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

    // Clear the selection component from the previously selected entity
    if (_serviceLocator.gameState.selectedEntity.has_value()) {
        auto oldSelectedEntity = _serviceLocator.gameState.selectedEntity.value();
        if (registry.valid(oldSelectedEntity)) {
            registry.remove<SelectedComponent>(oldSelectedEntity);
        }
    }

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
        registry.emplace<SelectedComponent>(clickedEntity); // Add component to new selection
        LOG_INFO("SelectionSystem", "Entity %u selected.", entt::to_integral(clickedEntity));
    } else {
        // If we clicked nothing, clear the selection state.
        // The component was already removed at the start of the function.
        if (_serviceLocator.gameState.selectedEntity.has_value()) {
            LOG_INFO("SelectionSystem", "Selection cleared.");
        }
        _serviceLocator.gameState.selectedEntity = std::nullopt;
    }
}