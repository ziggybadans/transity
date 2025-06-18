#include "SystemManager.h"

// The constructor just needs to store the ServiceLocator reference.
SystemManager::SystemManager(ServiceLocator& serviceLocator)
    : m_serviceLocator(serviceLocator) {}

// The update method now iterates through all registered systems
// and calls their update method. This simplifies the main game loop.
void SystemManager::update(sf::Time dt) {
    for (auto const& [type, system] : m_systems) {
        system->update(dt);
    }
}
