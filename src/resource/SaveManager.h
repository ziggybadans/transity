#pragma once

#include <string>
#include <filesystem>
#include <chrono>
#include <nlohmann/json.hpp>
#include "../settings/GameSettings.h"
#include "../world/Map.h"

class SaveManager {
public:
    SaveManager();
    ~SaveManager() = default;

    /* Save/Load Methods */
    bool SaveGame(const std::string& saveName);
    bool LoadGame(const std::string& saveName);
    bool QuickSave();
    bool QuickLoad();
    bool AutoSave();

    /* Save Management */
    bool DeleteSave(const std::string& saveName);
    std::vector<std::string> GetSaveList() const;
    bool SaveExists(const std::string& saveName) const;

    /* Settings */
    void SetAutosaveInterval(unsigned int minutes) { m_autosaveInterval = minutes; }
    void EnableAutosave(bool enable) { m_autosaveEnabled = enable; }

    /* Game State Access */
    void SetGameSettings(std::shared_ptr<GameSettings> settings) { m_gameSettings = settings; }
    void SetWorld(std::shared_ptr<Map> world) { m_world = world; }

private:
    /* Helper Methods */
    std::string GetSavePath(const std::string& saveName) const;
    bool SerializeGameState(nlohmann::json& j) const;
    bool DeserializeGameState(const nlohmann::json& j);
    void UpdateLastSaveTime();
    bool ShouldAutosave() const;

    /* Core Components */
    std::shared_ptr<GameSettings> m_gameSettings;
    std::shared_ptr<Map> m_world;  // Pointer to the game world

    /* Save Settings */
    std::filesystem::path m_savesDirectory;
    std::chrono::system_clock::time_point m_lastSaveTime;
    unsigned int m_autosaveInterval;
    bool m_autosaveEnabled;
    static constexpr const char* QUICKSAVE_NAME = "quicksave";
    static constexpr const char* AUTOSAVE_NAME = "autosave";
}; 