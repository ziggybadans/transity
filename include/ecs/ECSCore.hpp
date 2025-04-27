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
        return _registry.get<T>(entity);
    }

    template <typename T>
    const T& getComponent(entt::entity entity) const {
        return _registry.get<T>(entity);
    }
private:
    entt::registry _registry;
};

}