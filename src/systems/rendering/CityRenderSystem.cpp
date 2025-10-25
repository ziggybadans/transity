#include "CityRenderSystem.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"
#include "app/InteractionMode.h"
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

void CityRenderSystem::render(entt::registry& registry, sf::RenderTarget& target, const GameState& gameState, const sf::Color& highlightColor) {
    std::unordered_map<entt::entity, int> waitingCounts;
    auto passengerView = registry.view<const PassengerComponent>();
    for (auto passengerEntity : passengerView) {
        const auto& passenger = passengerView.get<const PassengerComponent>(passengerEntity);
        if (passenger.state == PassengerState::WAITING_FOR_TRAIN) {
            waitingCounts[passenger.currentContainer]++;
        }
    }

    auto cityView = registry.view<const CityComponent, const PositionComponent, const RenderableComponent>();
    for (auto entity : cityView) {
        const auto& city = cityView.get<const CityComponent>(entity);
        const auto& position = cityView.get<const PositionComponent>(entity);
        const auto& renderable = cityView.get<const RenderableComponent>(entity);

        switch (city.type) {
            case CityType::CAPITAL:
                renderCapital(target, position, renderable, gameState, highlightColor);
                break;
            case CityType::TOWN:
                renderTown(target, position, renderable, gameState, highlightColor);
                break;
            case CityType::SUBURB:
                renderSuburb(target, position, renderable, gameState, highlightColor);
                break;
        }

        if (waitingCounts.count(entity) > 0) {
            int count = waitingCounts.at(entity);
            m_text.setString(std::to_string(count));
            sf::FloatRect textBounds = m_text.getLocalBounds();
            m_text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                              textBounds.position.y + textBounds.size.y / 2.0f});
            m_text.setPosition(position.coordinates);
            target.draw(m_text);
        }
    }
}

void CityRenderSystem::renderCapital(sf::RenderTarget& target, const PositionComponent& position, const RenderableComponent& renderable, const GameState& gameState, const sf::Color& highlightColor) {
    float borderThickness = 4.0f;
    float size = renderable.radius.value * 2.0f;

    sf::RectangleShape border;
    border.setSize({size, size});
    border.setFillColor(highlightColor);
    border.setOrigin({renderable.radius.value, renderable.radius.value});
    border.setPosition(position.coordinates);
    target.draw(border);

    sf::RectangleShape innerSquare;
    float innerSize = (renderable.radius.value - borderThickness) * 2.0f;
    innerSquare.setSize({innerSize, innerSize});

    sf::Color innerColor = renderable.color;
    if (gameState.currentInteractionMode == InteractionMode::CREATE_LINE) {
        innerColor.a = 128;
    }
    innerSquare.setFillColor(innerColor);

    innerSquare.setOrigin({innerSize / 2.0f, innerSize / 2.0f});
    innerSquare.setPosition(position.coordinates);
    target.draw(innerSquare);
}

void CityRenderSystem::renderTown(sf::RenderTarget& target, const PositionComponent& position, const RenderableComponent& renderable, const GameState& gameState, const sf::Color& highlightColor) {
    float borderThickness = 4.0f;

    sf::CircleShape border(renderable.radius.value);
    border.setFillColor(highlightColor);
    border.setOrigin({renderable.radius.value, renderable.radius.value});
    border.setPosition(position.coordinates);
    target.draw(border);

    sf::CircleShape innerCircle(renderable.radius.value - borderThickness);

    sf::Color innerColor = renderable.color;
    if (gameState.currentInteractionMode == InteractionMode::CREATE_LINE) {
        innerColor.a = 128;
    }
    innerCircle.setFillColor(innerColor);

    innerCircle.setOrigin({renderable.radius.value - borderThickness,
                           renderable.radius.value - borderThickness});
    innerCircle.setPosition(position.coordinates);
    target.draw(innerCircle);
}

void CityRenderSystem::renderSuburb(sf::RenderTarget& target, const PositionComponent& position, const RenderableComponent& renderable, const GameState& gameState, const sf::Color& highlightColor) {
    float borderThickness = 4.0f;

    sf::CircleShape border(renderable.radius.value, 3);
    border.setFillColor(highlightColor);
    border.setOrigin({renderable.radius.value, renderable.radius.value});
    border.setPosition(position.coordinates);
    target.draw(border);

    sf::CircleShape innerTriangle(renderable.radius.value - borderThickness, 3);
    
    sf::Color innerColor = renderable.color;
    if (gameState.currentInteractionMode == InteractionMode::CREATE_LINE) {
        innerColor.a = 128;
    }
    innerTriangle.setFillColor(innerColor);

    innerTriangle.setOrigin({renderable.radius.value - borderThickness,
                             renderable.radius.value - borderThickness});
    innerTriangle.setPosition(position.coordinates);
    target.draw(innerTriangle);
}