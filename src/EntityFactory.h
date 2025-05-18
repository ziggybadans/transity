#pragma once

#include <entt/entt.hpp>
#include <SFML/System.hpp>
#include <string>

#include "Components.h" 

class EntityFactory {
public:
    EntityFactory(entt::registry& registry);

    entt::entity createStation(const sf::Vector2f& position, const std::string& name);

private:
    entt::registry& m_registry;
};