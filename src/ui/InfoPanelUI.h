#pragma once

#include "app/GameState.h"
#include "event/EventBus.h"
#include "event/UIEvents.h"
#include <entt/entt.hpp>
#include <optional>

class InfoPanelUI {
public:
    InfoPanelUI(entt::registry &registry, EventBus &eventBus, GameState &gameState);
    ~InfoPanelUI();

    void draw(float worldGenBottomY);

private:
    void onEntitySelected(const EntitySelectedEvent &event);
    void onEntityDeselected(const EntityDeselectedEvent &event);

    // Add these new helper functions
    void drawCityInfo(entt::entity entity);
    void drawTrainInfo(entt::entity entity);
    void drawLineInfo(entt::entity entity);
    void drawPassengerInfo(entt::entity entity);

    entt::registry &_registry;
    EventBus &_eventBus;
    GameState &_gameState;
    std::optional<entt::entity> _selectedEntity;

    entt::scoped_connection _entitySelectedConnection;
    entt::scoped_connection _entityDeselectedConnection;
};
