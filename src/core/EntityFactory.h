#pragma once

#include <entt/entt.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <map>
#include <optional>

#include "Components.h" 

namespace entityArchetypeData {
    struct RenderableData {
        float radius;
        sf::Color color;
    };

    struct ClickableData {
        float boundingRadius;
    };
}

class EntityFactory {
public:
    struct Archetype {
        std::string id;
        std::optional<entityArchetypeData::RenderableData> renderableData;
        std::optional<entityArchetypeData::ClickableData> clickableData;
    };

    EntityFactory(entt::registry& registry);

    entt::entity createStation(const sf::Vector2f& position, const std::string& name);
    entt::entity createLine(const std::vector<entt::entity>& stops, const sf::Color& color);

private:
    void registerArchetypes();
    void applyArchetype(entt::entity entity, const Archetype& archetype, const sf::Vector2f& position, const std::string& name);

    entt::registry& _registry;
    std::map<std::string, Archetype> _archetypes;
};