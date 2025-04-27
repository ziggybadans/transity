/**
 * @file ISystem.h
 * @brief Defines the base interfaces for systems within the ECS architecture.
 */
#pragma once

#include <entt/entt.hpp>

// Forward declaration for sf::RenderTarget to avoid including the full SFML header.
namespace sf { class RenderTarget; }

/**
 * @brief Namespace for the Entity-Component-System related classes and functions.
 */
namespace transity::ecs {

/**
 * @class ISystem
 * @brief Base interface for all systems in the ECS.
 * Provides a common virtual destructor for derived system types.
 */
class ISystem {
public:
    /**
     * @brief Virtual destructor. Ensures proper cleanup when deleting through a base pointer.
     */
    virtual ~ISystem() = default;
};

/**
 * @class IUpdateSystem
 * @brief Interface for systems that perform updates based on game logic and time.
 * Inherits from ISystem.
 */
class IUpdateSystem : public ISystem {
public:
    /**
     * @brief Pure virtual function to update the system's state.
     * @param registry A reference to the entt registry for accessing entities and components.
     * @param deltaTime The time elapsed since the last update, in seconds.
     */
    virtual void update(entt::registry& registry, float deltaTime) = 0;
};

/**
 * @class IRenderSystem
 * @brief Interface for systems responsible for rendering entities and components.
 * Inherits from ISystem.
 */
class IRenderSystem : public ISystem {
public:
    /**
     * @brief Pure virtual function to render the system's relevant entities/components.
     * @param registry A reference to the entt registry for accessing entities and components.
     * @param renderTarget The SFML render target to draw onto.
     */
    virtual void render(entt::registry& registry, sf::RenderTarget& renderTarget) = 0;
};

}