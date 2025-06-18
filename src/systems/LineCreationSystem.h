#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include "../core/SystemManager.h" // Include for ISystem
#include "../event/LineEvents.h"

// Forward declarations
class ServiceLocator;

class LineCreationSystem : public ISystem {
public:
    // Constructor now takes the ServiceLocator
    explicit LineCreationSystem(ServiceLocator& serviceLocator);
    ~LineCreationSystem();

    void clearCurrentLine();
    std::vector<entt::entity> getActiveLineStations() const;

private:
    // Event handlers
    void onAddStationToLine(const AddStationToLineEvent& event);
    void onFinalizeLine(const FinalizeLineEvent& event);

    void addStationToLine(entt::entity stationEntity);
    void finalizeLine();

    // Pointers to services obtained from the ServiceLocator
    entt::registry* _registry;
    class EntityFactory* _entityFactory;
    class ColorManager* _colorManager;

    // Event connections
    entt::connection m_addStationConnection;
    entt::connection m_finalizeLineConnection;
};
