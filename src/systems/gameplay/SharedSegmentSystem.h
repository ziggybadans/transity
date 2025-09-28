#pragma once

#include "ecs/ISystem.h"
#include "event/EventBus.h"
#include "event/LineEvents.h"
#include <entt/entt.hpp>

class SharedSegmentSystem : public ISystem {
public:
    SharedSegmentSystem(entt::registry& registry, EventBus& eventBus);
    ~SharedSegmentSystem();

private:
    void onLineModified(const LineModifiedEvent& event);
    void processSharedSegments();

    entt::registry& _registry;
    EventBus& _eventBus;
    entt::scoped_connection _lineModifiedConnection;
};