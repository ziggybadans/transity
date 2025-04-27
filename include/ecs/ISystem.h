#pragma once

#include <entt/entt.hpp>

namespace sf { class RenderTarget; }

namespace transity::ecs {

class ISystem {
public:
    virtual ~ISystem() = default;
};

class IUpdateSystem : public ISystem {
public:
    virtual void update(entt::registry& registry, float deltaTime) = 0;
};

class IRenderSystem : public ISystem {
public:
    virtual void render(entt::registry& registry, sf::RenderTarget& renderTarget) = 0;
};

}