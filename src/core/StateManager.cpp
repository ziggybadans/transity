#include "StateManager.h"
#include <stdexcept>
#include <iostream>

void StateManager::RegisterState(const std::string& stateName) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_states.find(stateName) == m_states.end()) {
        m_states[stateName] = std::any();
        m_observers[stateName] = std::vector<StateChangeCallback>();
    }
}

void StateManager::SetState(const std::string& stateName, const std::any& data) {
    std::vector<StateChangeCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        auto stateIt = m_states.find(stateName);
        if (stateIt == m_states.end()) {
            throw std::runtime_error("Attempting to set unregistered state: " + stateName);
        }
        
        stateIt->second = data;
        callbacks = m_observers[stateName];
    }
    
    // Notify observers outside of the lock to prevent deadlocks
    for (const auto& callback : callbacks) {
        try {
            callback(data);
        } catch (const std::exception& e) {
            // Log error but continue notifying other observers
            std::cerr << "Error in state change callback for state " << stateName 
                      << ": " << e.what() << std::endl;
        }
    }
}

void StateManager::Subscribe(const std::string& stateName, StateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto stateIt = m_states.find(stateName);
    if (stateIt == m_states.end()) {
        throw std::runtime_error("Attempting to subscribe to unregistered state: " + stateName);
    }
    
    m_observers[stateName].push_back(std::move(callback));
}

template<typename T>
T StateManager::GetState(const std::string& stateName) const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto stateIt = m_states.find(stateName);
    if (stateIt == m_states.end()) {
        throw std::runtime_error("Attempting to get unregistered state: " + stateName);
    }
    
    try {
        return std::any_cast<T>(stateIt->second);
    } catch (const std::bad_any_cast& e) {
        throw std::runtime_error("Type mismatch when getting state " + stateName + 
                               ": " + e.what());
    }
}

// Explicit template instantiations for common types
template bool StateManager::GetState<bool>(const std::string&) const;
template int StateManager::GetState<int>(const std::string&) const;
template float StateManager::GetState<float>(const std::string&) const;
template double StateManager::GetState<double>(const std::string&) const;
template std::string StateManager::GetState<std::string>(const std::string&) const;
template GameState StateManager::GetState<GameState>(const std::string&) const; 