#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "components/GameLogicComponents.h"
#include "components/RenderComponents.h"

class EntityFactory {
public:
    EntityFactory(entt::registry &registry);

    void loadArchetypes(const std::string &directoryPath);
    entt::entity createEntity(const std::string &archetypeId, const sf::Vector2f &position,
                          const std::string &name = "");
    entt::entity createLine(const std::vector<entt::entity> &stops, const sf::Color &color);
    entt::entity createTrain(entt::entity lineEntity);

private:
    entt::registry &_registry;
    std::map<std::string, nlohmann::json> _archetypes;
};