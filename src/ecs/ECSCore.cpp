/**
 * @file ECSCore.cpp
 * @brief Implements the core class for the Entity-Component-System (ECS) architecture.
 */
#include "ecs/ECSCore.hpp"
#include "logging/LoggingSystem.hpp"

namespace transity::ecs {

/**
 * @brief Initializes the ECS core. Logs an informational message.
 */
void ECSCore::initialize() {
    LOG_INFO("ECS", "ECS core initialized");
}

/**
 * @brief Creates a new entity within the registry. Logs the creation event.
 * @return The identifier of the newly created entity.
 */
entt::entity ECSCore::createEntity() {
    auto entity = _registry.create();
    LOG_DEBUG("ECS", "Created entity {}", static_cast<uint32_t>(entity));
    return entity;
}

/**
 * @brief Checks if the given entity identifier is valid within the registry.
 * @param entity The entity identifier to validate.
 * @return True if the entity is valid, false otherwise.
 */
bool ECSCore::hasEntity(entt::entity entity) {
    return _registry.valid(entity);
}

/**
 * @brief Destroys the specified entity, removing it and all its associated components from the registry.
 * Logs the destruction event.
 * @param entity The entity identifier to destroy.
 */
void ECSCore::destroyEntity(entt::entity entity) {
    LOG_DEBUG("ECS", "Destroying entity {}", static_cast<uint32_t>(entity));
    _registry.destroy(entity);
}

/**
 * @brief Gets the number of currently active (alive) entities in the registry.
 * @return The count of active entities.
 */
size_t ECSCore::getEntityCount() const {
    return _registry.alive();
}

/**
 * @brief Registers an update system. The system will be executed during the update phase.
 * Logs the registration event.
 * @param system A unique pointer to an IUpdateSystem implementation.
 */
void ECSCore::registerUpdateSystem(std::unique_ptr<IUpdateSystem> system) {
    if (system) {
        // TODO: Log system type name if possible without RTTI overhead?
        LOG_INFO("ECS", "Registering Update System");
        _updateSystems.push_back(std::move(system));
    }
}

/**
 * @brief Registers a render system. The system will be executed during the render phase.
 * Logs the registration event.
 * @param system A unique pointer to an IRenderSystem implementation.
 */
void ECSCore::registerRenderSystem(std::unique_ptr<IRenderSystem> system) {
    if (system) {
        // TODO: Log system type name if possible without RTTI overhead?
        LOG_INFO("ECS", "Registering Render System");
        _renderSystems.push_back(std::move(system));
    }
}

/**
 * @brief Executes the update method of all registered update systems.
 * Iterates through the list of update systems and calls their update function.
 * @param deltaTime The time elapsed since the last update cycle.
 */
void ECSCore::updateSystems(float deltaTime) {
    for (auto& system : _updateSystems) {
        system->update(_registry, deltaTime);
    }
}

/**
 * @brief Executes the render method of all registered render systems.
 * Iterates through the list of render systems and calls their render function.
 * @param renderTarget The target to render onto (e.g., sf::RenderWindow).
 */
void ECSCore::renderSystems(sf::RenderTarget& renderTarget) {
    for (auto& system : _renderSystems) {
        system->render(_registry, renderTarget);
    }
}

/**
 * @brief Shuts down the ECS core. Clears all systems and the entity registry.
 * Logs the shutdown event.
 */
void ECSCore::shutdown() {
    _updateSystems.clear();
    _renderSystems.clear();
    _registry.clear();
    LOG_INFO("ECS", "ECS core shut down");
}

}