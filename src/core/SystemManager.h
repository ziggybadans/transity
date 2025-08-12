#pragma once

#include "../Logger.h"
#include "ISystem.h"
#include "ServiceLocator.h"
#include <SFML/System/Time.hpp>
#include <memory>
#include <typeindex>
#include <unordered_map>

class SystemManager {
public:
    
    explicit SystemManager(ServiceLocator &serviceLocator);

    
    
    
    template <typename T, typename... Args> T *addSystem(Args &&...args) {
        
        static_assert(std::is_base_of<ISystem, T>::value, "System must derive from ISystem");

        auto system = std::make_unique<T>(m_serviceLocator, std::forward<Args>(args)...);
        T *ptr = system.get();  
        m_systems[std::type_index(typeid(T))] = std::move(system);
        LOG_INFO("SystemManager", "Added system: %s", typeid(T).name());
        return ptr;
    }

    
    template <typename T> T *getSystem() {
        auto it = m_systems.find(std::type_index(typeid(T)));
        if (it != m_systems.end()) {
            
            return static_cast<T *>(it->second.get());
        }
        return nullptr;  
    }

    
    void update(sf::Time dt);

private:
    ServiceLocator &m_serviceLocator;
    
    std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;
};
