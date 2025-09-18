#pragma once

#include "ecs/ISystem.h"
#include "event/InputEvents.h"

class ServiceLocator;

class SelectionSystem : public ISystem {
public:
    explicit SelectionSystem(ServiceLocator& serviceLocator);
    ~SelectionSystem() override;

    void onMouseButtonPressed(const MouseButtonPressedEvent& event);

private:
    ServiceLocator& _serviceLocator;
    entt::scoped_connection _mouseButtonConnection;
};