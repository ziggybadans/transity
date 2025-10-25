#include "TrainRenderSystem.h"
#include "components/GameLogicComponents.h"
#include "components/TrainComponents.h"
#include "components/RenderComponents.h"
#include "Logger.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <stdexcept>

sf::Font TrainRenderSystem::loadFont() {
    sf::Font font;
    if (!font.openFromFile("data/fonts/font.TTF")) {
        const std::string errorMsg = "Failed to load font: data/fonts/font.TTF";
        LOG_ERROR("TrainRenderSystem", errorMsg.c_str());
        throw std::runtime_error(errorMsg);
    }
    return font;
}

TrainRenderSystem::TrainRenderSystem()
    : m_font(loadFont()),
      m_text(m_font)
{
    m_text.setCharacterSize(18);
    m_text.setFillColor(sf::Color::White);
}

void TrainRenderSystem::render(const entt::registry &registry, sf::RenderTarget &target, const sf::Color& highlightColor) {
    auto view = registry.view<const PositionComponent, const RenderableComponent, const TrainTag, const TrainCapacityComponent>();
    for (auto entity : view) {
        const auto &pos = view.get<const PositionComponent>(entity);
        const auto &renderable = view.get<const RenderableComponent>(entity);
        const auto &capacity = view.get<const TrainCapacityComponent>(entity);

        sf::CircleShape shape(renderable.radius.value);
        shape.setFillColor(renderable.color);
        shape.setOrigin({renderable.radius.value, renderable.radius.value});
        shape.setPosition(pos.coordinates);
        
        target.draw(shape);

        if (capacity.currentLoad > 0) {
            m_text.setString(std::to_string(capacity.currentLoad));
            sf::FloatRect textBounds = m_text.getLocalBounds();
            m_text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                              textBounds.position.y + textBounds.size.y / 2.0f});
            m_text.setPosition(pos.coordinates);
            target.draw(m_text);
        }

        if (registry.all_of<SelectedComponent>(entity)) {
            sf::CircleShape highlight(renderable.radius.value + 3.0f);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(highlightColor);
            highlight.setOutlineThickness(2.0f);
            highlight.setOrigin({renderable.radius.value + 3.0f, renderable.radius.value + 3.0f});
            highlight.setPosition(pos.coordinates);
            target.draw(highlight);
        }
    }
}