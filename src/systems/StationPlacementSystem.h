// src/systems/StationPlacementSystem.h

#pragma once

#include "entt/entt.hpp"
#include "../core/SystemManager.h" // Include for ISystem
#include "../event/InputEvents.h"

// Forward declarations
class ServiceLocator;

class StationPlacementSystem : public ISystem {
public:
    explicit StationPlacementSystem(ServiceLocator& serviceLocator);
    ~StationPlacementSystem();

private:
    // This handler is replaced by onMouseButtonPressed
    // void onTryPlaceStation(const TryPlaceStationEvent& event);
    
    // New handler for the generic mouse press event
    void onMouseButtonPressed(const MouseButtonPressedEvent& event);

    // Pointers to services obtained from the ServiceLocator
    entt::registry* _registry;
    class EntityFactory* _entityFactory;
    class GameState* _gameState;
    
    // Connection for the old event is removed
    // entt::connection m_placeStationConnection;

    // Connection for the new event handler
    entt::connection m_mousePressConnection;
};
