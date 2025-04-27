/**
 * @file ECSCore.hpp
 * @brief Defines the core class for the Entity-Component-System (ECS) architecture.
 */
#pragma once

#include <entt/entt.hpp>

#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <typeinfo> // Required for typeid

#include "ecs/ISystem.h"
#include "logging/LoggingSystem.hpp" // Required for logging macros

/**
 * @brief Namespace for the Entity-Component-System related classes and functions.
 */
namespace transity::ecs {

/**
 * @class ECSCore
 * @brief Manages entities, components, and systems within the ECS architecture.
 *
 * This class serves as the central hub for the ECS, handling the lifecycle
 * of entities, the association of components, and the registration and
 * execution of systems. It utilizes the 'entt' library for the underlying
 * registry management.
 */
class ECSCore {
public:
    /**
     * @brief Initializes the ECS core.
     * Placeholder for any necessary setup. Currently does nothing.
     */
    void initialize();

    /**
     * @brief Creates a new entity.
     * @return The newly created entity identifier.
     */
    entt::entity createEntity();

    /**
     * @brief Checks if an entity exists and is valid.
     * @param entity The entity identifier to check.
     * @return True if the entity exists and is valid, false otherwise.
     */
    bool hasEntity(entt::entity);

    /**
     * @brief Destroys an existing entity and removes all its components.
     * @param entity The entity identifier to destroy.
     */
    void destroyEntity(entt::entity);

    /**
     * @brief Gets the current number of active entities.
     * @return The total count of entities managed by the registry.
     */
    size_t getEntityCount() const;

    /**
     * @brief Registers a system that updates game logic.
     * @param system A unique pointer to an object implementing IUpdateSystem.
     */
    void registerUpdateSystem(std::unique_ptr<IUpdateSystem> system);

    /**
     * @brief Registers a system responsible for rendering.
     * @param system A unique pointer to an object implementing IRenderSystem.
     */
    void registerRenderSystem(std::unique_ptr<IRenderSystem> system);

    /**
     * @brief Executes the update logic for all registered update systems.
     * @param deltaTime The time elapsed since the last frame, in seconds.
     */
    void updateSystems(float deltaTime);

    /**
     * @brief Executes the rendering logic for all registered render systems.
     * @param renderTarget The SFML render target (e.g., sf::RenderWindow) to draw on.
     */
    void renderSystems(sf::RenderTarget& renderTarget);

    /**
     * @brief Shuts down the ECS core, clearing the registry and systems.
     */
    void shutdown();

    /**
     * @brief Adds a component of type T to the specified entity or replaces it if it already exists.
     * @tparam T The type of the component to add.
     * @tparam Args The types of arguments for the component's constructor.
     * @param entity The entity to add the component to.
     * @param args Arguments to forward to the component's constructor.
     * @return A reference to the added or replaced component.
     */
    template<typename T, typename... Args>
    T& addComponent(entt::entity entity, Args&&... args) {
        LOG_DEBUG("ECS", "Adding component '{}' to entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
        return _registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    /**
     * @brief Checks if an entity has a component of type T.
     * @tparam T The type of the component to check for.
     * @param entity The entity to check.
     * @return True if the entity has the component, false otherwise.
     */
    template <typename T>
    bool hasComponent(entt::entity entity) const {
        return _registry.all_of<T>(entity);
    }

    /**
     * @brief Gets a reference to a component of type T for the specified entity.
     * @tparam T The type of the component to retrieve.
     * @param entity The entity whose component is to be retrieved.
     * @return A mutable reference to the component.
     * @throws std::runtime_error if the entity does not have the specified component.
     */
    template <typename T>
    T& getComponent(entt::entity entity) {
        if (!hasComponent<T>(entity)) {
            LOG_WARN("ECS", "Attempted to get non-existent component '{}' from entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
            throw std::runtime_error("Entity does not have the specified component.");
        }
        return _registry.get<T>(entity);
    }

    /**
     * @brief Gets a constant reference to a component of type T for the specified entity.
     * @tparam T The type of the component to retrieve.
     * @param entity The entity whose component is to be retrieved.
     * @return A constant reference to the component.
     * @throws std::runtime_error if the entity does not have the specified component.
     * @note This is the const-qualified overload of getComponent.
     */
    template <typename T>
    const T& getComponent(entt::entity entity) const {
        if (!hasComponent<T>(entity)) {
            LOG_WARN("ECS", "Attempted to get non-existent component '{}' from entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
            throw std::runtime_error("Entity does not have the specified component.");
        }
        return _registry.get<T>(entity);
    }

    /**
     * @brief Removes a component of type T from the specified entity.
     * If the entity does not have the component, this function does nothing.
     * @tparam T The type of the component to remove.
     * @param entity The entity from which to remove the component.
     */
    template<typename T>
    void removeComponent(entt::entity entity) {
        LOG_DEBUG("ECS", "Removing component '{}' from entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
        _registry.remove<T>(entity);
    }

    /**
     * @brief Gets a view of entities that possess all the specified component types.
     * Views are lightweight objects used to iterate over entities and their components.
     * @tparam ComponentTs The component types to include in the view.
     * @return An entt::view for the specified component types.
     */
    template<typename ...ComponentTs>
    auto getView() const {
        return _registry.view<ComponentTs...>();
    }
private:
    entt::registry _registry; ///< The underlying entt registry managing entities and components.
    std::vector<std::unique_ptr<IUpdateSystem>> _updateSystems; ///< Collection of registered update systems.
    std::vector<std::unique_ptr<IRenderSystem>> _renderSystems; ///< Collection of registered render systems.
};

}