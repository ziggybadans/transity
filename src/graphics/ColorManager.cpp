#include "ColorManager.h"

ColorManager::ColorManager() : _currentLineColorIndex(0) {
    _lineColors = { sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan };
}

sf::Color ColorManager::getNextLineColor() {
    sf::Color color = _lineColors[_currentLineColorIndex];
    _currentLineColorIndex = (_currentLineColorIndex + 1) % _lineColors.size();
    return color;
}