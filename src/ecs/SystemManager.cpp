#include "SystemManager.h"

SystemManager::SystemManager(ServiceLocator &serviceLocator) : m_serviceLocator(serviceLocator) {}

void SystemManager::update(sf::Time dt) {
    for (auto system : m_updatableSystems) {
        system->update(dt);
    }
}