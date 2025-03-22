#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <type_traits>
#include <iostream>
#include "transity/core/system.hpp"

namespace transity {
namespace core {

/**
 * @brief Manages all subsystems in the engine
 * 
 * Handles registration, initialization, updating, and shutdown of all systems.
 * Systems are updated in priority order and can be enabled/disabled at runtime.
 */
class SystemManager {
public:
    /**
     * @brief Register a new system
     * @tparam T System type (must inherit from ISystem)
     * @param priority Update priority for the system
     * @return Pointer to the registered system
     */
    template<typename T>
    T* registerSystem(int priority = 0) {
        static_assert(std::is_base_of<ISystem, T>::value, "System must inherit from ISystem");

        auto system = std::make_unique<T>();
        system->setPriority(priority);
        
        T* systemPtr = system.get();
        m_systems.push_back(std::move(system));
        m_systemsByName[systemPtr->getName()] = systemPtr;

        // Sort systems by priority
        sortSystems();
        
        return systemPtr;
    }

    /**
     * @brief Get a system by type
     * @tparam T System type to retrieve
     * @return Pointer to the system or nullptr if not found
     */
    template<typename T>
    T* getSystem() {
        static_assert(std::is_base_of<ISystem, T>::value, "System must inherit from ISystem");

        for (const auto& system : m_systems) {
            if (auto typed = dynamic_cast<T*>(system.get())) {
                return typed;
            }
        }
        return nullptr;
    }

    /**
     * @brief Get a system by name
     * @param name Name of the system to retrieve
     * @return Pointer to the system or nullptr if not found
     */
    ISystem* getSystem(const std::string& name) {
        auto it = m_systemsByName.find(name);
        return it != m_systemsByName.end() ? it->second : nullptr;
    }

    /**
     * @brief Initialize all registered systems
     * @return true if all systems initialized successfully
     */
    bool initialize() {
        for (const auto& system : m_systems) {
            if (!system->initialize()) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Update all enabled systems
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) {
        for (const auto& system : m_systems) {
            if (system->isEnabled()) {
                system->update(deltaTime);
            }
        }
    }

    /**
     * @brief Shut down all systems and cleanup
     */
    void shutdown() {
        // Shutdown in reverse order of initialization
        for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
            (*it)->shutdown();
        }
        m_systems.clear();
        m_systemsByName.clear();
    }

private:
    void sortSystems() {
        std::sort(m_systems.begin(), m_systems.end(),
            [](const auto& a, const auto& b) {
                return a->getPriority() < b->getPriority();
            });
    }

    std::vector<std::unique_ptr<ISystem>> m_systems;
    std::unordered_map<std::string, ISystem*> m_systemsByName;
};

} // namespace core
} // namespace transity 