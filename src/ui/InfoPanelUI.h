#pragma once

#include "event/EventBus.h"
#include "event/UIEvents.h"
#include <entt/entt.hpp>
#include <optional>

class InfoPanelUI {
public:
    InfoPanelUI(entt::registry &registry, EventBus &eventBus);
    ~InfoPanelUI();

    void draw();

private:
    void onEntitySelected(const EntitySelectedEvent &event);
    void onEntityDeselected(const EntityDeselectedEvent &event);

    entt::registry &_registry;
    EventBus &_eventBus;
    std::optional<entt::entity> _selectedEntity;

    entt::scoped_connection _entitySelectedConnection;
    entt::scoped_connection _entityDeselectedConnection;
};