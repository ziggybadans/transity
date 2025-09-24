#include "SystemManager.h"

void SystemManager::update(sf::Time dt) {
    for (auto system : m_updatableSystems) {
        system->update(dt);
    }
}