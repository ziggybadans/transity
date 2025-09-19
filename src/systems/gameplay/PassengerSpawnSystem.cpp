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
            return; // Not enough connected cities to create a passenger trip
        }

        const int maxAttempts = 10;
        for (int i = 0; i < maxAttempts; ++i) {
            // Use a more robust random selection
            std::random_device rd;
            std::mt19937 gen(rd());
            std::shuffle(connectedCities.begin(), connectedCities.end(), gen);

            entt::entity originCity = connectedCities[0];
            entt::entity destinationCity = connectedCities[1];

            // Find a path for the passenger
            std::vector<entt::entity> path = _pathfinder.findPath(originCity, destinationCity);

            if (!path.empty()) {
                entt::entity passengerEntity = _entityFactory.createPassenger(originCity, destinationCity);
                if (_registry.valid(passengerEntity)) {
                    // Get the existing PathComponent and populate it
                    auto& pathComponent = _registry.get<PathComponent>(passengerEntity);
                    pathComponent.nodes = path;
                    pathComponent.currentNodeIndex = 0;
                    LOG_DEBUG("PassengerSpawnSystem", "Passenger created with a path of %zu stops from %u to %u.", path.size(), entt::to_integral(originCity), entt::to_integral(destinationCity));
                }
                return; // Passenger spawned, exit for this tick
            }
        }

        LOG_WARN("PassengerSpawnSystem", "Failed to find a valid path for a passenger after %d attempts.", maxAttempts);
    }
}