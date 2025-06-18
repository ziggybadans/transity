#pragma once

#include <SFML/System/Time.hpp>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "ServiceLocator.h"
#include "../Logger.h"

// A base class for all systems to provide a common interface.
class ISystem {
public:
    virtual ~ISystem() = default;
    // Optional update method for systems that need logic to run every frame.
    virtual void update(sf::Time /*dt*/) {}
};

class SystemManager {
public:
    // The constructor now takes the ServiceLocator, which it will use to construct systems.
    explicit SystemManager(ServiceLocator& serviceLocator);

    // Templated method to add a new system.
    // It constructs the system of type T, passing the ServiceLocator to its constructor,
    // and then stores it.
    template<typename T>
    T* addSystem() {
        // Ensure T is derived from ISystem
        static_assert(std::is_base_of<ISystem, T>::value, "System must derive from ISystem");

        auto system = std::make_unique<T>(m_serviceLocator);
        T* ptr = system.get(); // Get raw pointer before moving ownership
        m_systems[std::type_index(typeid(T))] = std::move(system);
        LOG_INFO("SystemManager", "Added system: %s", typeid(T).name());
        return ptr;
    }

    // Templated method to retrieve a system by its type.
    template<typename T>
    T* getSystem() {
        auto it = m_systems.find(std::type_index(typeid(T)));
        if (it != m_systems.end()) {
            // static_cast is safe here because we know the type from the map key.
            return static_cast<T*>(it->second.get());
        }
        return nullptr; // System not found
    }

    // The update method will now iterate over all registered systems and call their update method.
    void update(sf::Time dt);

private:
    ServiceLocator& m_serviceLocator;
    // The manager now holds a map of systems, keyed by their type information.
    std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;
};
