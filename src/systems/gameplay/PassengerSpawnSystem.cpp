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
      _pathfinder(serviceLocator.pathfinder),
      _spawnInterval(sf::seconds(5.0f)), // Spawn a passenger every 5 seconds
      _spawnTimer(_spawnInterval) {
    LOG_DEBUG("PassengerSpawnSystem", "PassengerSpawnSystem created.");
}

void PassengerSpawnSystem::update(sf::Time dt) {
    _spawnTimer -= dt;

    if (_spawnTimer.asSeconds() <= 0.0f) {
        _spawnTimer = _spawnInterval;

        auto cityView = _registry.view<const CityComponent>();
        std::vector<entt::entity> connectedCities;
        for (auto entity : cityView) {
            if (!cityView.get<const CityComponent>(entity).connectedLines.empty()) {
                connectedCities.push_back(entity);
            }
        }

        if (connectedCities.size() < 2) {
            return;
        }

        const int maxAttempts = 10;
        for (int i = 0; i < maxAttempts; ++i) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::shuffle(connectedCities.begin(), connectedCities.end(), gen);

            entt::entity originCity = connectedCities[0];
            entt::entity destinationCity = connectedCities[1];

            std::vector<entt::entity> path = _pathfinder.findPath(originCity, destinationCity);

            if (!path.empty()) {
                entt::entity passengerEntity = _entityFactory.createPassenger(originCity, destinationCity);
                if (_registry.valid(passengerEntity)) {
                    auto& pathComponent = _registry.get<PathComponent>(passengerEntity);
                    pathComponent.nodes = path;
                    pathComponent.currentNodeIndex = 0;

                    // Add passenger to the origin city's waiting list
                    auto& originCityComponent = _registry.get<CityComponent>(originCity);
                    originCityComponent.waitingPassengers.push_back(passengerEntity);

                    LOG_DEBUG("PassengerSpawnSystem", "Passenger %u created at city %u, waiting for train. Path size: %zu.", entt::to_integral(passengerEntity), entt::to_integral(originCity), path.size());
                }
                return;
            }
        }

        LOG_WARN("PassengerSpawnSystem", "Failed to find a valid path for a passenger after %d attempts.", maxAttempts);
    }
}