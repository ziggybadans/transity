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

size_t ECSCore::getEntityCount() const {
    return _registry.alive();
}

void ECSCore::registerUpdateSystem(std::unique_ptr<IUpdateSystem> system) {
    if (system) {
        _updateSystems.push_back(std::move(system));
    }
}

void ECSCore::registerRenderSystem(std::unique_ptr<IRenderSystem> system) {
    if (system) {
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