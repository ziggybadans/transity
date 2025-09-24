#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"
#include "event/LineEvents.h"
#include "app/GameState.h"
#include "event/EventBus.h"
#include <SFML/Graphics/Color.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <functional>

class EntityFactory;
class ColorManager;

class LineCreationSystem : public ISystem, public IUpdatable {
public:
    explicit LineCreationSystem(entt::registry& registry, EntityFactory& entityFactory, ColorManager& colorManager, GameState& gameState, EventBus& eventBus);
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
    EventBus& _eventBus;

    entt::scoped_connection m_finalizeLineConnection;
    entt::scoped_connection m_mousePressConnection;
    entt::scoped_connection m_cancelLineCreationConnection;
};