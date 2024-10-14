// src/entities/EntityManager.h
#pragma once

#include "Entity.h"
#include <queue>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <cassert>
#include <vector>

class ComponentBase {
public:
    virtual ~ComponentBase() = default;
};

template <typename T>
class Component : public ComponentBase {
public:
    T data;
};

class EntityManager {
public:
    // Creates a new entity and returns it, either by reusing an available ID or by generating a new one
    Entity createEntity();

    // Destroys an entity by marking its ID as available for reuse and removing its components
    void destroyEntity(EntityID id);

    // Adds a component to an entity
    template <typename T>
    void addComponent(EntityID id, const T& component) {
        std::type_index typeIndex(typeid(T));
        // Ensure the component map exists for this type
        if (components.find(typeIndex) == components.end()) {
            components[typeIndex] = std::unordered_map<EntityID, std::unique_ptr<ComponentBase>>();
        }
        // Add or replace the component for this entity
        components[typeIndex][id] = std::make_unique<Component<T>>(Component<T>{component});
    }

    // Retrieves a pointer to a component of a specific type from an entity
    template <typename T>
    T* getComponent(EntityID id) {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            auto compIt = it->second.find(id);
            if (compIt != it->second.end()) {
                return &(static_cast<Component<T>*>(compIt->second.get())->data);
            }
        }
        return nullptr;
    }

    // Removes a component of a specific type from an entity
    template <typename T>
    void removeComponent(EntityID id) {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            it->second.erase(id);
        }
    }

private:
    std::queue<EntityID> availableIDs;  // Queue to hold reusable entity IDs
    EntityID nextID = 0;  // The next ID to be used if no reusable IDs are available

    // Maps to store components of each type
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::unique_ptr<ComponentBase>>> components;
};

// Implementation of non-template methods

inline Entity EntityManager::createEntity() {
    EntityID id;
    if (!availableIDs.empty()) {
        id = availableIDs.front();
        availableIDs.pop();
    }
    else {
        id = nextID++;
    }
    return Entity(id);
}

inline void EntityManager::destroyEntity(EntityID id) {
    // Remove all components associated with this entity
    for (auto& [type, componentMap] : components) {
        componentMap.erase(id);
    }
    // Mark the ID as available for reuse
    availableIDs.push(id);
}
