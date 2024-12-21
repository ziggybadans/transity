#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <string>
#include <any>

enum class GameState {
    MainMenu,
    Paused,
    Running,
    Loading
};

class StateManager {
public:
    using StateChangeCallback = std::function<void(const std::any&)>;

    void InitializeCoreStates();
    void RegisterState(const std::string& stateName);
    void SetState(const std::string& stateName, const std::any& data = std::any());
    void Subscribe(const std::string& stateName, StateChangeCallback callback);
    
    template<typename T>
    T GetState(const std::string& stateName) const;

    std::string GameStateToString(GameState state) {
        switch (state) {
        case GameState::MainMenu: return "MainMenu";
        case GameState::Loading: return "Loading";
        case GameState::Running: return "Running";
        case GameState::Paused: return "Paused";
        default: throw std::invalid_argument("Unknown GameState");
        }
    }

    GameState StringToGameState(const std::string& stateName) {
        if (stateName == "MainMenu") return GameState::MainMenu;
        if (stateName == "Loading") return GameState::Loading;
        if (stateName == "Running") return GameState::Running;
        if (stateName == "Paused") return GameState::Paused;
        throw std::invalid_argument("Unknown state name: " + stateName);
    }

private:
    std::unordered_map<std::string, std::any> m_states;
    std::unordered_map<std::string, std::vector<StateChangeCallback>> m_observers;
    mutable std::mutex m_stateMutex;
}; 