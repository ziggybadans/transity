#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <string>
#include <any>

enum class GameState {
    MainMenu,
    Loading,
    Running,
    Paused,
    Saving
};

class StateManager {
public:
    using StateChangeCallback = std::function<void(const std::any&)>;

    void RegisterState(const std::string& stateName);
    void SetState(const std::string& stateName, const std::any& data = std::any());
    void Subscribe(const std::string& stateName, StateChangeCallback callback);
    
    template<typename T>
    T GetState(const std::string& stateName) const;

private:
    std::unordered_map<std::string, std::any> m_states;
    std::unordered_map<std::string, std::vector<StateChangeCallback>> m_observers;
    mutable std::mutex m_stateMutex;
}; 