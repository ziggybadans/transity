#pragma once

#include <entt/entt.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <map>
#include <optional>

#include "Components.h" 

namespace EntityArchetypeData {
    struct RenderableData {
        float radius;
        sf::Color color;
    };
}

class EntityFactory {
public:
    struct Archetype {
        std::string id; 
        bool has_station_tag = false;
        std::optional<EntityArchetypeData::RenderableData> renderable_data;
    };

    EntityFactory(entt::registry& registry);

    entt::entity createStation(const sf::Vector2f& position, const std::string& name);
    entt::entity createLine(const std::vector<entt::entity>& stops, const sf::Color& color);

private:
    void registerArchetypes(); 
    void applyArchetype(entt::entity entity, const Archetype& archetype, const sf::Vector2f& position, const std::string& name);

    entt::registry& m_registry;
    std::map<std::string, Archetype> m_archetypes;
};