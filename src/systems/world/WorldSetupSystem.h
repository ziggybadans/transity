#pragma once

#include "../core/ISystem.h"

class ServiceLocator;

class WorldSetupSystem : public ISystem, public IUpdatable {
public:
    explicit WorldSetupSystem(ServiceLocator &services);
    ~WorldSetupSystem() override = default;

    void init();
    void update(sf::Time dt) override;

private:
    ServiceLocator &m_services;
};
