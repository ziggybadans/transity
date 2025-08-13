#pragma once

#include <SFML/System/Time.hpp>

class ISystem {
public:
    virtual ~ISystem() = default;
};

class IUpdatable {
public:
    virtual ~IUpdatable() = default;
    virtual void update(sf::Time dt) = 0;
};
