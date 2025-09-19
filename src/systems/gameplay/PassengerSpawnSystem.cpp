#include "PassengerSpawnSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "Logger.h"
#include <vector>
#include <random>

PassengerSpawnSystem::PassengerSpawnSystem(ServiceLocator& serviceLocator)
    : _registry(serviceLocator.registry),
      _entityFactory(serviceLocator.entityFactory),
      _spawnInterval(sf::seconds(5.0f)), // Spawn a passenger every 5 seconds
      _spawnTimer(_spawnInterval) {
    LOG_DEBUG("PassengerSpawnSystem", "PassengerSpawnSystem created.");
}

void PassengerSpawnSystem::update(sf::Time dt) {
    _spawnTimer -= dt;

    if (_spawnTimer.asSeconds() <= 0.0f) {
        _spawnTimer = _spawnInterval;

        auto cityView = _registry.view<CityComponent>();
        std::vector<entt::entity> cities;
        for (auto entity : cityView) {
            cities.push_back(entity);
        }

        if (cities.size() < 2) {
            // Not enough cities to create a passenger trip
            return;
        }

        // Simple random selection for origin and destination
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, static_cast<int>(cities.size() - 1));

        int originIndex = distrib(gen);
        int destinationIndex;
        do {
            destinationIndex = distrib(gen);
        } while (originIndex == destinationIndex);

        entt::entity originCity = cities[originIndex];
        entt::entity destinationCity = cities[destinationIndex];

        _entityFactory.createPassenger(originCity, destinationCity);
    }
}