#pragma once

#include "entt/entt.hpp"
#include "../core/SystemManager.h" // Include for ISystem
#include "../event/InputEvents.h"

// Forward declarations
class ServiceLocator;

class StationPlacementSystem : public ISystem {
public:
    // Constructor now takes the ServiceLocator
    explicit StationPlacementSystem(ServiceLocator& serviceLocator);
    ~StationPlacementSystem();

private:
    void onTryPlaceStation(const TryPlaceStationEvent& event);

    // Pointers to services obtained from the ServiceLocator
    entt::registry* _registry;
    class EntityFactory* _entityFactory;
    class GameState* _gameState;
    
    entt::connection m_placeStationConnection;
};
