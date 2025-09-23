#pragma once
#include "event/EventBus.h"
#include "event/UIEvents.h"
#include <SFML/Graphics/Color.hpp>
#include <map>
#include <vector>

struct ThemeColors {
    sf::Color backgroundColor;
    sf::Color landColor;
    sf::Color waterColor;
    sf::Color riverColor;
    std::vector<sf::Color> lineColors;
};

class ColorManager {
public:
    ColorManager(EventBus &eventBus);
    void setTheme(Theme theme);
    Theme getTheme() const;

    const sf::Color &getBackgroundColor() const;
    const sf::Color &getLandColor() const;
    const sf::Color &getWaterColor() const;
    const sf::Color &getRiverColor() const;
    sf::Color getNextLineColor() noexcept;

private:
    EventBus &_eventBus;
    Theme _activeTheme;
    std::map<Theme, ThemeColors> _themes;
    int _currentLineColorIndex;
};