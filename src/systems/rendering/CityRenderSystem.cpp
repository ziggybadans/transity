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
        const auto& city = cityView.get<const CityComponent>(entity);
        const auto& position = cityView.get<const PositionComponent>(entity);
        const auto& renderable = cityView.get<const RenderableComponent>(entity);

        switch (city.type) {
            case CityType::CAPITAL:
                renderCapital(window, position, renderable, highlightColor);
                break;
            case CityType::TOWN:
                renderTown(window, position, renderable, highlightColor);
                break;
            case CityType::SUBURB:
                renderSuburb(window, position, renderable, highlightColor);
                break;
        }

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

void CityRenderSystem::renderCapital(sf::RenderWindow& window, const PositionComponent& position, const RenderableComponent& renderable, const sf::Color& highlightColor) {
    float borderThickness = 4.0f;
    float size = renderable.radius.value * 2.0f;

    sf::RectangleShape border;
    border.setSize({size, size});
    border.setFillColor(highlightColor);
    border.setOrigin({renderable.radius.value, renderable.radius.value});
    border.setPosition(position.coordinates);
    window.draw(border);

    sf::RectangleShape innerSquare;
    float innerSize = (renderable.radius.value - borderThickness) * 2.0f;
    innerSquare.setSize({innerSize, innerSize});
    innerSquare.setFillColor(renderable.color);
    innerSquare.setOrigin({innerSize / 2.0f, innerSize / 2.0f});
    innerSquare.setPosition(position.coordinates);
    window.draw(innerSquare);
}

void CityRenderSystem::renderTown(sf::RenderWindow& window, const PositionComponent& position, const RenderableComponent& renderable, const sf::Color& highlightColor) {
    float borderThickness = 4.0f;

    sf::CircleShape border(renderable.radius.value);
    border.setFillColor(highlightColor);
    border.setOrigin({renderable.radius.value, renderable.radius.value});
    border.setPosition(position.coordinates);
    window.draw(border);

    sf::CircleShape innerCircle(renderable.radius.value - borderThickness);
    innerCircle.setFillColor(renderable.color);
    innerCircle.setOrigin({renderable.radius.value - borderThickness,
                           renderable.radius.value - borderThickness});
    innerCircle.setPosition(position.coordinates);
    window.draw(innerCircle);
}

void CityRenderSystem::renderSuburb(sf::RenderWindow& window, const PositionComponent& position, const RenderableComponent& renderable, const sf::Color& highlightColor) {
    float borderThickness = 4.0f;

    sf::CircleShape border(renderable.radius.value, 3);
    border.setFillColor(highlightColor);
    border.setOrigin({renderable.radius.value, renderable.radius.value});
    border.setPosition(position.coordinates);
    window.draw(border);

    sf::CircleShape innerTriangle(renderable.radius.value - borderThickness, 3);
    innerTriangle.setFillColor(renderable.color);
    innerTriangle.setOrigin({renderable.radius.value - borderThickness,
                             renderable.radius.value - borderThickness});
    innerTriangle.setPosition(position.coordinates);
    window.draw(innerTriangle);
}