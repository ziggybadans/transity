#pragma once

#include "ecs/ISystem.h"
#include "core/ServiceLocator.h"
#include "entt/entt.hpp"
#include "event/EventBus.h"
#include "event/LineEvents.h"

class EntityFactory;

class LineDataSystem : public ISystem, public IUpdatable {
public:
    LineDataSystem(ServiceLocator& serviceLocator);
    ~LineDataSystem();
    void update(sf::Time dt) override;

private:
    void processParallelSegments();
    void onAddTrain(const AddTrainToLineEvent& event);

    entt::registry& _registry;
    EntityFactory& _entityFactory;
    entt::connection m_addTrainConnection;
};