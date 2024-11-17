#include "SaveManager.h"
#include <fstream>
#include <iostream>
#include "../Debug.h"

SaveManager::SaveManager()
    : m_savesDirectory("saves")
    , m_autosaveInterval(5)
    , m_autosaveEnabled(true)
{
    std::filesystem::create_directories(m_savesDirectory);
    m_lastSaveTime = std::chrono::system_clock::now();
}

bool SaveManager::SaveGame(const std::string& saveName) {
    try {
        DEBUG_INFO("Saving game to: ", saveName);
        nlohmann::json saveData;
        if (!SerializeGameState(saveData)) {
            DEBUG_ERROR("Failed to serialize game state");
            return false;
        }

        std::filesystem::path savePath = GetSavePath(saveName);
        std::ofstream saveFile(savePath);
        saveFile << saveData.dump(4);
        
        UpdateLastSaveTime();
        DEBUG_INFO("Game saved successfully");
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error saving game: ", e.what());
        return false;
    }
}

bool SaveManager::LoadGame(const std::string& saveName) {
    try {
        std::filesystem::path savePath = GetSavePath(saveName);
        if (!std::filesystem::exists(savePath)) {
            std::cerr << "Save file does not exist: " << savePath << std::endl;
            return false;
        }

        std::ifstream saveFile(savePath);
        nlohmann::json saveData;
        saveFile >> saveData;

        return DeserializeGameState(saveData);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading game: " << e.what() << std::endl;
        return false;
    }
}

bool SaveManager::QuickSave() {
    return SaveGame(QUICKSAVE_NAME);
}

bool SaveManager::QuickLoad() {
    return LoadGame(QUICKSAVE_NAME);
}

bool SaveManager::AutoSave() {
    if (!m_autosaveEnabled || !ShouldAutosave()) {
        return false;
    }
    return SaveGame(AUTOSAVE_NAME);
}

bool SaveManager::DeleteSave(const std::string& saveName) {
    try {
        std::filesystem::path savePath = GetSavePath(saveName);
        return std::filesystem::remove(savePath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting save: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> SaveManager::GetSaveList() const {
    std::vector<std::string> saves;
    for (const auto& entry : std::filesystem::directory_iterator(m_savesDirectory)) {
        if (entry.path().extension() == ".save") {
            saves.push_back(entry.path().stem().string());
        }
    }
    return saves;
}

bool SaveManager::SaveExists(const std::string& saveName) const {
    return std::filesystem::exists(GetSavePath(saveName));
}

std::string SaveManager::GetSavePath(const std::string& saveName) const {
    return (m_savesDirectory / (saveName + ".save")).string();
}

bool SaveManager::SerializeGameState(nlohmann::json& j) const {
    if (!m_gameSettings) {
        return false;
    }

    try {
        // Serialize game settings
        j["settings"] = {
            {"resolution", {m_gameSettings->GetResolution().x, m_gameSettings->GetResolution().y}},
            {"fullscreen", m_gameSettings->IsFullscreen()},
            {"vsync", m_gameSettings->IsVSyncEnabled()},
            {"frameRateLimit", m_gameSettings->GetFrameRateLimit()},
            {"masterVolume", m_gameSettings->GetMasterVolume()},
            {"musicVolume", m_gameSettings->GetMusicVolume()},
            {"sfxVolume", m_gameSettings->GetSFXVolume()},
            {"cameraZoomSpeed", m_gameSettings->GetCameraZoomSpeed()},
            {"cameraPanSpeed", m_gameSettings->GetCameraPanSpeed()},
            {"autosaveInterval", m_gameSettings->GetAutosaveInterval()}
        };

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error serializing game state: " << e.what() << std::endl;
        return false;
    }
}

bool SaveManager::DeserializeGameState(const nlohmann::json& j) {
    if (!m_gameSettings) {
        return false;
    }

    try {
        // Deserialize settings
        auto& settings = j["settings"];
        m_gameSettings->SetResolution(sf::Vector2u(settings["resolution"][0], settings["resolution"][1]));
        m_gameSettings->SetFullscreen(settings["fullscreen"]);
        m_gameSettings->SetVSync(settings["vsync"]);
        m_gameSettings->SetFrameRateLimit(settings["frameRateLimit"]);
        m_gameSettings->SetMasterVolume(settings["masterVolume"]);
        m_gameSettings->SetMusicVolume(settings["musicVolume"]);
        m_gameSettings->SetSFXVolume(settings["sfxVolume"]);
        m_gameSettings->SetCameraZoomSpeed(settings["cameraZoomSpeed"]);
        m_gameSettings->SetCameraPanSpeed(settings["cameraPanSpeed"]);
        m_gameSettings->SetAutosaveInterval(settings["autosaveInterval"]);

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deserializing game state: " << e.what() << std::endl;
        return false;
    }
}

void SaveManager::UpdateLastSaveTime() {
    m_lastSaveTime = std::chrono::system_clock::now();
}

bool SaveManager::ShouldAutosave() const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - m_lastSaveTime);
    return duration.count() >= m_autosaveInterval;
} 