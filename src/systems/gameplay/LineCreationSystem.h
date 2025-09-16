

#pragma once

#include "ecs/ISystem.h"
#include "ecs/SystemManager.h"
#include "event/InputEvents.h"
#include "event/LineEvents.h"
#include <SFML/Graphics/Color.hpp>
#include <entt/entt.hpp>
#include <vector>

class ServiceLocator;
class GameState;

class LineCreationSystem : public ISystem, public IUpdatable {
public:
    explicit LineCreationSystem(ServiceLocator &serviceLocator);
    ~LineCreationSystem();

    void update(sf::Time dt) override;

    void clearCurrentLine() noexcept;
    void getActiveLineStations(std::function<void(entt::entity)> callback) const noexcept;

private:
    void onFinalizeLine(const FinalizeLineEvent &event);
    void onMouseButtonPressed(const MouseButtonPressedEvent &event);
    void onCancelLineCreation(const CancelLineCreationEvent &event);

    void addStationToLine(entt::entity stationEntity);
    void finalizeLine();

    entt::registry &_registry;
    EntityFactory &_entityFactory;
    ColorManager &_colorManager;
    GameState &_gameState;

    entt::connection m_finalizeLineConnection;
    entt::connection m_mousePressConnection;
    entt::connection m_cancelLineCreationConnection;
};
