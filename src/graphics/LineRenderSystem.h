#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>

class LineRenderSystem {
public:
    void render(const entt::registry &registry, sf::RenderWindow &window, const sf::View &view);

private:
    std::vector<std::pair<int, entt::entity>> m_taggedStationsPairs;
    std::vector<entt::entity> m_activeLineStations;
};
