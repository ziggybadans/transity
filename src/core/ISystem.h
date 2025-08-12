
#pragma once
#include <SFML/System/Time.hpp>

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(sf::Time dt) = 0;
};
