#pragma once

#include <vector>
#include <entt/entt.hpp>
#include "LineEvents.h"
#include "EntityFactory.h"

class LineCreationSystem {
public:
    LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory);
    
    void addStationToLine(entt::entity stationEntity);
    void finalizeLine();
    void clearCurrentLine();
    std::vector<entt::entity> getActiveLineStations() const;
    void processEvents(const std::vector<std::variant<AddStationToLineEvent, 
        FinalizeLineEvent>>& gameEvents, const std::vector<FinalizeLineEvent>& uiEvents);

private:
    entt::registry& _registry;
    EntityFactory _entityFactory;
};