#include "transity/ecs/entity_manager.hpp"
#include <algorithm>

namespace transity::ecs {

EntityManager::EntityManager() : nextEntityId(0) {}

Entity EntityManager::createEntity() {
    EntityID id;
    
    // Use recycled ID if available, otherwise use next sequential ID
    if (!recycledIds.empty()) {
        id = recycledIds.front();
        recycledIds.pop();
    } else {
        id = nextEntityId++;
    }

    // For recycled IDs, version should already be set and we increment it
    // For new IDs, we initialize version to 1
    auto& version = versions[id];
    if (version == 0) {
        version = 1;
    } else {
        version++;
    }

    Entity entity{id, version};
    activeEntities.push_back(entity);
    return entity;
}

bool EntityManager::destroyEntity(Entity entity) {
    if (!isValid(entity)) {
        return false;
    }

    // Remove from active entities
    auto it = std::find_if(activeEntities.begin(), activeEntities.end(),
        [entity](const Entity& e) { return e.id == entity.id; });
    
    if (it != activeEntities.end()) {
        activeEntities.erase(it);
        recycledIds.push(entity.id);
        // Increment version to invalidate the entity immediately
        versions[entity.id]++;
        return true;
    }

    return false;
}

bool EntityManager::isValid(Entity entity) const {
    auto it = versions.find(entity.id);
    return it != versions.end() && it->second == entity.version;
}

size_t EntityManager::getEntityCount() const {
    return activeEntities.size();
}

const std::vector<Entity>& EntityManager::getActiveEntities() const {
    return activeEntities;
}

} // namespace transity::ecs 