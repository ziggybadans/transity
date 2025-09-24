#pragma once

#include "ecs/ISystem.h"
#include "entt/entt.hpp"
#include "event/LineEvents.h"
#include "event/EventBus.h"

class EntityFactory;

class LineDataSystem : public ISystem, public IUpdatable {
public:
    LineDataSystem(entt::registry& registry, EntityFactory& entityFactory, EventBus& eventBus);
    ~LineDataSystem();
    void update(sf::Time dt) override;

private:
    void processParallelSegments();
    void onAddTrain(const AddTrainToLineEvent& event);

    entt::registry& _registry;
    EntityFactory& _entityFactory;
    entt::scoped_connection m_addTrainConnection;
};