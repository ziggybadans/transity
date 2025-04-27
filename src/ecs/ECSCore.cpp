#include "ecs/ECSCore.hpp"
#include "logging/LoggingSystem.hpp"

namespace transity::ecs {

void ECSCore::initialize() {
    LOG_INFO("ECS", "ECS core initialized");
}

entt::entity ECSCore::createEntity() {
    return _registry.create();
}

bool ECSCore::hasEntity(entt::entity entity) {
    return _registry.valid(entity);
}

void ECSCore::destroyEntity(entt::entity entity) {
    _registry.destroy(entity);
}

void ECSCore::shutdown() {
}

}