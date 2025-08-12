#include "SystemManager.h"


SystemManager::SystemManager(ServiceLocator &serviceLocator) : m_serviceLocator(serviceLocator) {}



void SystemManager::update(sf::Time dt) {
    for (auto const &[type, system] : m_systems) {
        system->update(dt);
    }
}
