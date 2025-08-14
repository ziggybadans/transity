#pragma once
#include <SFML/Graphics/Color.hpp>
#include <vector>

class ColorManager {
public:
    ColorManager();
    sf::Color getNextLineColor() noexcept;

private:
    std::vector<sf::Color> _lineColors;
    int _currentLineColorIndex;
};