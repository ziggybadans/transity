#pragma once

#include <functional>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../graphics/Camera.h"

class Map;

class InputCommand {
public:
    virtual ~InputCommand() = default;
    virtual void execute() = 0;
};

/**
 * A generic command that obtains the mouse position in world coordinates,
 * and then calls a user-provided action on the Map.
 */
class MapInteractionCommand : public InputCommand {
public:
    using MapAction = std::function<void(std::shared_ptr<Map>, const sf::Vector2f&)>;

    MapInteractionCommand(std::shared_ptr<Camera> camera,
        sf::RenderWindow& window,
        std::shared_ptr<Map> map,
        MapAction action)
        : m_camera(camera)
        , m_window(window)
        , m_map(map)
        , m_action(std::move(action))
    {}

    void execute() override {
        // 1. Get the mouse position in pixel coordinates
        sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);

        // 2. Convert it to world coordinates
        sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos, m_camera->GetView());

        // 3. Call the user-provided action (e.g. Place, Draw, Select, Move, etc.)
        if (m_map && m_action) {
            m_action(m_map, worldPos);
        }
    }

private:
    std::shared_ptr<Camera> m_camera;
    sf::RenderWindow& m_window;
    std::shared_ptr<Map>    m_map;
    MapAction               m_action;
};

