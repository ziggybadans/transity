#include "CityRenderSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"
#include <stdexcept>

sf::Font CityRenderSystem::loadFont() {
    sf::Font font;
    if (!font.openFromFile("data/fonts/font.TTF")) {
        const std::string errorMsg = "Failed to load font: data/fonts/font.TTF";
        LOG_ERROR("CityRenderSystem", errorMsg.c_str());
        throw std::runtime_error(errorMsg);
    }
    return font;
}

CityRenderSystem::CityRenderSystem() : m_font(loadFont()), m_text(m_font) {
    m_text.setCharacterSize(24);
    m_text.setFillColor(sf::Color::Black);
}
void CityRenderSystem::render(entt::registry& registry, sf::RenderWindow& window, const sf::Color& highlightColor) {
    auto cityView = registry.view<const CityComponent, const PositionComponent, const RenderableComponent>();

    for (auto entity : cityView) {
        const auto& position = cityView.get<const PositionComponent>(entity);
        const auto& renderable = cityView.get<const RenderableComponent>(entity);

        float borderThickness = 4.0f;

        // Draw the outer border
        sf::CircleShape border(renderable.radius.value);
        border.setFillColor(highlightColor);
        border.setOrigin({renderable.radius.value, renderable.radius.value});
        border.setPosition(position.coordinates);
        window.draw(border);

        // Draw the inner circle
        sf::CircleShape innerCircle(renderable.radius.value - borderThickness);
        innerCircle.setFillColor(renderable.color);
        innerCircle.setOrigin({renderable.radius.value - borderThickness,
                               renderable.radius.value - borderThickness});
        innerCircle.setPosition(position.coordinates);
        window.draw(innerCircle);

        // Draw passenger count for cities
        int waitingCount = 0;
        auto passengerView = registry.view<const PassengerComponent>();
        for (auto passengerEntity : passengerView) {
            const auto& passenger = passengerView.get<const PassengerComponent>(passengerEntity);
            if (passenger.currentContainer == entity) {
                waitingCount++;
            }
        }

        if (waitingCount > 0) {
            m_text.setString(std::to_string(waitingCount));
            sf::FloatRect textBounds = m_text.getLocalBounds();
            m_text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                              textBounds.position.y + textBounds.size.y / 2.0f});
            m_text.setPosition(position.coordinates);
            window.draw(m_text);
        }
    }
}