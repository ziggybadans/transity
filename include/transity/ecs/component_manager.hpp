#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <typeindex>
#include <vector>
#include <cassert>
#include <functional>

namespace transity::ecs {

class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void remove(size_t index) = 0;
};

template<typename T>
class ComponentPool : public IComponentPool {
public:
    T& create(size_t index) {
        if (index >= m_components.size()) {
            m_components.resize(index + 1);
        }
        m_activeComponents.insert(index);
        return m_components[index];
    }

    void remove(size_t index) override {
        assert(index < m_components.size());
        m_components[index] = T{};
        m_activeComponents.erase(index);
    }

    T* get(size_t index) {
        if (index >= m_components.size() || m_activeComponents.find(index) == m_activeComponents.end()) {
            return nullptr;
        }
        return &m_components[index];
    }

    const T* get(size_t index) const {
        if (index >= m_components.size() || m_activeComponents.find(index) == m_activeComponents.end()) {
            return nullptr;
        }
        return &m_components[index];
    }

private:
    std::vector<T> m_components;
    std::unordered_set<size_t> m_activeComponents;
};

class ComponentManager {
public:
    template<typename T>
    T& addComponent(size_t entityId) {
        auto& pool = getComponentPool<T>();
        T& component = pool.create(entityId);
        
        // Register component type for this entity
        m_entityComponents[entityId].insert(std::type_index(typeid(T)));
        
        if (m_onComponentAdded) {
            m_onComponentAdded(entityId, std::type_index(typeid(T)));
        }
        
        return component;
    }

    template<typename T>
    void removeComponent(size_t entityId) {
        auto& pool = getComponentPool<T>();
        pool.remove(entityId);
        
        // Remove component type registration
        auto it = m_entityComponents.find(entityId);
        if (it != m_entityComponents.end()) {
            it->second.erase(std::type_index(typeid(T)));
            if (it->second.empty()) {
                m_entityComponents.erase(it);
            }
        }
        
        if (m_onComponentRemoved) {
            m_onComponentRemoved(entityId, std::type_index(typeid(T)));
        }
    }

    template<typename T>
    T* getComponent(size_t entityId) {
        auto& pool = getComponentPool<T>();
        return pool.get(entityId);
    }

    template<typename T>
    const T* getComponent(size_t entityId) const {
        auto& pool = getComponentPool<T>();
        return pool.get(entityId);
    }

    bool hasComponent(size_t entityId, const std::type_index& type) const {
        auto it = m_entityComponents.find(entityId);
        if (it == m_entityComponents.end()) {
            return false;
        }
        return it->second.find(type) != it->second.end();
    }

    template<typename T>
    bool hasComponent(size_t entityId) const {
        return hasComponent(entityId, std::type_index(typeid(T)));
    }

    void removeAllComponents(size_t entityId) {
        auto it = m_entityComponents.find(entityId);
        if (it != m_entityComponents.end()) {
            for (const auto& type : it->second) {
                if (m_onComponentRemoved) {
                    m_onComponentRemoved(entityId, type);
                }
            }
            m_entityComponents.erase(it);
        }
    }

    // Callback setters for component lifecycle events
    void setOnComponentAdded(std::function<void(size_t, std::type_index)> callback) {
        m_onComponentAdded = std::move(callback);
    }

    void setOnComponentRemoved(std::function<void(size_t, std::type_index)> callback) {
        m_onComponentRemoved = std::move(callback);
    }

private:
    template<typename T>
    ComponentPool<T>& getComponentPool() {
        auto typeIndex = std::type_index(typeid(T));
        auto it = m_componentPools.find(typeIndex);
        if (it == m_componentPools.end()) {
            auto pool = std::make_unique<ComponentPool<T>>();
            auto [insertIt, success] = m_componentPools.insert(
                {typeIndex, std::move(pool)}
            );
            return *static_cast<ComponentPool<T>*>(insertIt->second.get());
        }
        return *static_cast<ComponentPool<T>*>(it->second.get());
    }

    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_componentPools;
    std::unordered_map<size_t, std::unordered_set<std::type_index>> m_entityComponents;
    std::function<void(size_t, std::type_index)> m_onComponentAdded;
    std::function<void(size_t, std::type_index)> m_onComponentRemoved;
};

} // namespace transity::ecs 