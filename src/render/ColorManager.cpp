#include "ColorManager.h"

ColorManager::ColorManager(EventBus &eventBus)
    : _eventBus(eventBus), _activeTheme(Theme::Dark), _currentLineColorIndex(0) {
    // Define themes
    _themes[Theme::Light] = {sf::Color(173, 216, 230),  // backgroundColor
                             sf::Color(255, 255, 255),  // landColor
                             sf::Color(229, 240, 247),  // waterColor
                             sf::Color(100, 149, 237),  // riverColor
                             {                          // lineColors
                              sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow,
                              sf::Color::Magenta, sf::Color::Cyan}};

    _themes[Theme::Dark] = {sf::Color(25, 25, 25),    // backgroundColor
                            sf::Color(70, 70, 70),    // landColor
                            sf::Color(40, 40, 40),    // waterColor
                            sf::Color(60, 100, 180),  // riverColor
                            {                         // lineColors
                             sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow,
                             sf::Color::Magenta, sf::Color::Cyan}};

    _eventBus.trigger<ThemeChangedEvent>({_activeTheme});
}

void ColorManager::setTheme(Theme theme) {
    if (_activeTheme != theme) {
        _activeTheme = theme;
        _eventBus.trigger<ThemeChangedEvent>({theme});
    }
}

Theme ColorManager::getTheme() const {
    return _activeTheme;
}

const sf::Color &ColorManager::getBackgroundColor() const {
    return _themes.at(_activeTheme).backgroundColor;
}

const sf::Color &ColorManager::getLandColor() const {
    return _themes.at(_activeTheme).landColor;
}

const sf::Color &ColorManager::getWaterColor() const {
    return _themes.at(_activeTheme).waterColor;
}

const sf::Color &ColorManager::getRiverColor() const {
    return _themes.at(_activeTheme).riverColor;
}

sf::Color ColorManager::getNextLineColor() noexcept {
    const auto &lineColors = _themes.at(_activeTheme).lineColors;
    if (lineColors.empty()) {
        return sf::Color::White;  // Fallback color
    }
    sf::Color color = lineColors[_currentLineColorIndex];
    _currentLineColorIndex = (_currentLineColorIndex + 1) % lineColors.size();
    return color;
}