#include "ecs/ECSCore.hpp"
#include "logging/LoggingSystem.hpp"

namespace transity::ecs {

void ECSCore::initialize() {
    LOG_INFO("ECS", "ECS core initialized");
}

entt::entity ECSCore::createEntity() {
    auto entity = _registry.create();
    LOG_DEBUG("ECS", "Created entity {}", static_cast<uint32_t>(entity));
    return entity;
}

bool ECSCore::hasEntity(entt::entity entity) {
    return _registry.valid(entity);
}

void ECSCore::destroyEntity(entt::entity entity) {
    LOG_DEBUG("ECS", "Destroying entity {}", static_cast<uint32_t>(entity));
    _registry.destroy(entity);
}

size_t ECSCore::getEntityCount() const {
    return _registry.alive();
}

void ECSCore::registerUpdateSystem(std::unique_ptr<IUpdateSystem> system) {
    if (system) {
        // TODO: Log system type name if possible without RTTI overhead?
        LOG_INFO("ECS", "Registering Update System");
        _updateSystems.push_back(std::move(system));
    }
}

void ECSCore::registerRenderSystem(std::unique_ptr<IRenderSystem> system) {
    if (system) {
        // TODO: Log system type name if possible without RTTI overhead?
        LOG_INFO("ECS", "Registering Render System");
        _renderSystems.push_back(std::move(system));
    }
}

void ECSCore::updateSystems(float deltaTime) {
    for (auto& system : _updateSystems) {
        system->update(_registry, deltaTime);
    }
}

void ECSCore::renderSystems(sf::RenderTarget& renderTarget) {
    for (auto& system : _renderSystems) {
        system->render(_registry, renderTarget);
    }
}

void ECSCore::shutdown() {
    _updateSystems.clear();
    _renderSystems.clear();
    _registry.clear();
    LOG_INFO("ECS", "ECS core shut down");
}

}