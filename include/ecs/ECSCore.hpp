#pragma once

#include <entt/entt.hpp>

#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <typeinfo> // Required for typeid

#include "ecs/ISystem.h"
#include "logging/LoggingSystem.hpp" // Required for logging macros

namespace transity::ecs {

class ECSCore {
public:
    void initialize();

    entt::entity createEntity();
    bool hasEntity(entt::entity);
    void destroyEntity(entt::entity);
    size_t getEntityCount() const;

    void registerUpdateSystem(std::unique_ptr<IUpdateSystem> system);
    void registerRenderSystem(std::unique_ptr<IRenderSystem> system);

    void updateSystems(float deltaTime);
    void renderSystems(sf::RenderTarget& renderTarget);

    void shutdown();

    template<typename T, typename... Args>
    T& addComponent(entt::entity entity, Args&&... args) {
        LOG_DEBUG("ECS", "Adding component '{}' to entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
        return _registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    bool hasComponent(entt::entity entity) const {
        return _registry.all_of<T>(entity);
    }

    template <typename T>
    T& getComponent(entt::entity entity) {
        if (!hasComponent<T>(entity)) {
            LOG_WARN("ECS", "Attempted to get non-existent component '{}' from entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
            throw std::runtime_error("Entity does not have the specified component.");
        }
        return _registry.get<T>(entity);
    }

    template <typename T>
    const T& getComponent(entt::entity entity) const {
        if (!hasComponent<T>(entity)) {
            LOG_WARN("ECS", "Attempted to get non-existent component '{}' from entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
            throw std::runtime_error("Entity does not have the specified component.");
        }
        return _registry.get<T>(entity);
    }

    template<typename T>
    void removeComponent(entt::entity entity) {
        LOG_DEBUG("ECS", "Removing component '{}' from entity {}", typeid(T).name(), static_cast<uint32_t>(entity));
        _registry.remove<T>(entity);
    }

    template<typename ...ComponentTs>
    auto getView() const {
        return _registry.view<ComponentTs...>();
    }
private:
    entt::registry _registry;
    std::vector<std::unique_ptr<IUpdateSystem>> _updateSystems;
    std::vector<std::unique_ptr<IRenderSystem>> _renderSystems;
};

}