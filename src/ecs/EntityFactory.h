#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <functional> // Add this

#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"

struct TrainMovementComponent;
struct TrainPhysicsComponent;
struct TrainCapacityComponent;
struct PassengerComponent;

class EntityFactory {
public:
    EntityFactory(entt::registry &registry);

    void loadArchetypes(const std::string &directoryPath);
    entt::entity createEntity(const std::string &archetypeId, const sf::Vector2f &position,
                              const std::string &name = "");
    entt::entity createLine(const std::vector<entt::entity> &stops, const sf::Color &color);
    entt::entity createTrain(entt::entity lineEntity);
    entt::entity createPassenger(entt::entity origin, entt::entity destination);

private:
    void registerComponentFactories(); // Add this

    entt::registry &_registry;
    std::map<std::string, nlohmann::json> _archetypes;
    std::map<std::string, std::function<void(entt::entity, const nlohmann::json&)>> _componentFactories; // Add this
};