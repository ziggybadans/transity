#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include "../core/EntityFactory.h"
#include "../graphics/ColorManager.h"
#include "../event/LineEvents.h"
#include "../event/EventBus.h"

class LineCreationSystem {
public:
    // Constructor now takes the EventBus
    LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager, EventBus& eventBus);
    ~LineCreationSystem();

    // This method is no longer needed
    // void processEvents(const std::vector<std::variant<AddStationToLineEvent, FinalizeLineEvent>>& inputHandlerEvents, const std::vector<FinalizeLineEvent>& uiEvents);

    void clearCurrentLine();
    std::vector<entt::entity> getActiveLineStations() const;

private:
    // Event handlers
    void onAddStationToLine(const AddStationToLineEvent& event);
    void onFinalizeLine(const FinalizeLineEvent& event);

    void addStationToLine(entt::entity stationEntity);
    void finalizeLine();

    entt::registry& _registry;
    EntityFactory& _entityFactory;
    ColorManager& _colorManager;

    // Event connections
    entt::connection m_addStationConnection;
    entt::connection m_finalizeLineConnection;
};
