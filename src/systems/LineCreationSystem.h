// src/systems/LineCreationSystem.h

#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include "../core/SystemManager.h" // Include for ISystem
#include "../event/LineEvents.h"
#include "../event/InputEvents.h" // Add include for InputEvents

// Forward declarations
class ServiceLocator;
class GameState; // Forward-declare GameState

class LineCreationSystem : public ISystem {
public:
    explicit LineCreationSystem(ServiceLocator& serviceLocator);
    ~LineCreationSystem();

    void clearCurrentLine();
    std::vector<entt::entity> getActiveLineStations() const;

private:
    // Event handlers
    // void onAddStationToLine(const AddStationToLineEvent& event); // Replaced
    void onFinalizeLine(const FinalizeLineEvent& event);
    void onMouseButtonPressed(const MouseButtonPressedEvent& event); // New handler

    void addStationToLine(entt::entity stationEntity);
    void finalizeLine();

    // Pointers to services obtained from the ServiceLocator
    entt::registry* _registry;
    EntityFactory* _entityFactory;
    ColorManager* _colorManager;
    GameState* _gameState; // Add GameState service pointer

    // Event connections
    // entt::connection m_addStationConnection; // Replaced
    entt::connection m_finalizeLineConnection;
    entt::connection m_mousePressConnection; // New connection
};
