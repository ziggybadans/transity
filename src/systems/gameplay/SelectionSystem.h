#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"
#include "core/Pathfinder.h"

class ServiceLocator;

class SelectionSystem : public ISystem {
public:
    explicit SelectionSystem(ServiceLocator& serviceLocator);
    ~SelectionSystem() override;

    void onMouseButtonPressed(const MouseButtonPressedEvent& event);

private:
    ServiceLocator& _serviceLocator;
    Pathfinder& _pathfinder;
    entt::scoped_connection _mouseButtonConnection;
};