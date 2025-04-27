#pragma once

#include <entt/entt.hpp>
#include <utility>

namespace transity::ecs {

class ECSCore {
public:
    void initialize();
    void shutdown();

    entt::entity createEntity();
    bool hasEntity(entt::entity);
    void destroyEntity(entt::entity);

    template<typename T, typename... Args>
    T& addComponent(entt::entity entity, Args&&... args) {
        return _registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    bool hasComponent(entt::entity entity) const {
        return _registry.all_of<T>(entity);
    }

    template <typename T>
    T& getComponent(entt::entity entity) {
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have the specified component.");
        }
        return _registry.get<T>(entity);
    }

    template <typename T>
    const T& getComponent(entt::entity entity) const {
        if (!hasComponent<T>(entity)) {
            throw std::runtime_error("Entity does not have the specified component.");
        }
        return _registry.get<T>(entity);
    }

    template<typename T>
    void removeComponent(entt::entity entity) {
        _registry.remove<T>(entity);
    }
private:
    entt::registry _registry;
};

}