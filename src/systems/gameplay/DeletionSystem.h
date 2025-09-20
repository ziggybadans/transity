#pragma once

#include "ecs/ISystem.h"
#include "core/ServiceLocator.h"
#include "event/DeletionEvents.h"
#include <entt/entt.hpp>

class DeletionSystem : public ISystem {
public:
    DeletionSystem(ServiceLocator& serviceLocator);
    ~DeletionSystem();

private:
    void onDeleteEntity(const DeleteEntityEvent& event);

    ServiceLocator& _serviceLocator;
    entt::registry& _registry;
    entt::scoped_connection _deleteEntityConnection;
};