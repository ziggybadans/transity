#include "PassengerSpawnAnimationSystem.h"
#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"
#include "components/PassengerComponents.h"
#include "core/ServiceLocator.h"
#include "ecs/EntityFactory.h"
#include "core/Pathfinder.h"
#include "Logger.h"

PassengerSpawnAnimationSystem::PassengerSpawnAnimationSystem(ServiceLocator& serviceLocator)
    : _registry(serviceLocator.registry),
      _entityFactory(serviceLocator.entityFactory),
      _pathfinder(serviceLocator.pathfinder) {
    LOG_DEBUG("PassengerSpawnAnimationSystem", "PassengerSpawnAnimationSystem created.");
}

void PassengerSpawnAnimationSystem::update(sf::Time dt) {
    auto view = _registry.view<PassengerSpawnAnimationComponent>();
    for (auto entity : view) {
        auto& animation = view.get<PassengerSpawnAnimationComponent>(entity);
        animation.progress += dt.asSeconds() / animation.duration;

        if (animation.progress >= 1.0f) {
            // Animation finished, spawn the passenger
            std::vector<entt::entity> path = _pathfinder.findPath(animation.originCity, animation.destinationCity);
            if (!path.empty()) {
                entt::entity passengerEntity = _entityFactory.createPassenger(animation.originCity, animation.destinationCity);
                if (_registry.valid(passengerEntity)) {
                    auto& pathComponent = _registry.get<PathComponent>(passengerEntity);
                    pathComponent.nodes = path;
                    pathComponent.currentNodeIndex = 0;

                    auto& originCityComponent = _registry.get<CityComponent>(animation.originCity);
                    originCityComponent.waitingPassengers.push_back(passengerEntity);

                    LOG_DEBUG("PassengerSpawnAnimationSystem", "Passenger %u created at city %u after animation.", entt::to_integral(passengerEntity), entt::to_integral(animation.originCity));
                }
            } else {
                LOG_WARN("PassengerSpawnAnimationSystem", "Failed to find path for passenger after animation.");
            }

            _registry.remove<PassengerSpawnAnimationComponent>(entity);
        }
    }
}

void PassengerSpawnAnimationSystem::render(sf::RenderTarget& target) {
    auto view = _registry.view<const PassengerSpawnAnimationComponent, const PositionComponent, const RenderableComponent>();
    for (auto entity : view) {
        const auto& animation = view.get<const PassengerSpawnAnimationComponent>(entity);
        const auto& position = view.get<const PositionComponent>(entity);
        const auto& renderable = view.get<const RenderableComponent>(entity);

        sf::CircleShape circle(renderable.radius.value);
        circle.setOrigin({renderable.radius.value, renderable.radius.value});
        circle.setPosition(position.coordinates);

        // Draw the background
        circle.setFillColor(sf::Color(renderable.color.r, renderable.color.g, renderable.color.b, renderable.color.a));
        target.draw(circle);

        // Draw the fill animation
        float fillRadius = renderable.radius.value * (1.0f - animation.progress);
        sf::CircleShape fillCircle(fillRadius);
        fillCircle.setOrigin({fillRadius, fillRadius});
        fillCircle.setPosition(position.coordinates);
        fillCircle.setFillColor(sf::Color::Blue);
        target.draw(fillCircle);
    }
}