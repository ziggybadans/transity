#include "SaveManager.h"
#include <fstream>
#include <iostream>
#include "../Debug.h"
#include "../settings/SettingsDefinitions.h"
#include <SFML/Graphics.hpp>

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
        // Serialize settings
        j["settings"] = {
            {"resolution", {
                m_gameSettings->GetValue<sf::Vector2u>(Settings::Names::RESOLUTION).x,
                m_gameSettings->GetValue<sf::Vector2u>(Settings::Names::RESOLUTION).y
            }},
            {"fullscreen", m_gameSettings->GetValue<bool>(Settings::Names::FULLSCREEN)},
            {"vsync", m_gameSettings->GetValue<bool>(Settings::Names::VSYNC)},
            {"frameRateLimit", m_gameSettings->GetValue<unsigned int>(Settings::Names::FRAME_RATE_LIMIT)},
            {"masterVolume", m_gameSettings->GetValue<float>(Settings::Names::MASTER_VOLUME)},
            {"musicVolume", m_gameSettings->GetValue<float>(Settings::Names::MUSIC_VOLUME)},
            {"sfxVolume", m_gameSettings->GetValue<float>(Settings::Names::SFX_VOLUME)},
            {"cameraZoomSpeed", m_gameSettings->GetValue<float>(Settings::Names::CAMERA_ZOOM_SPEED)},
            {"cameraPanSpeed", m_gameSettings->GetValue<float>(Settings::Names::CAMERA_PAN_SPEED)},
            {"autosaveInterval", m_gameSettings->GetValue<unsigned int>(Settings::Names::AUTOSAVE_INTERVAL)}
        };

        return true;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error serializing game state: ", e.what());
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
        m_gameSettings->SetValue(Settings::Names::RESOLUTION, 
            sf::Vector2u(settings["resolution"][0], settings["resolution"][1]));
        m_gameSettings->SetValue(Settings::Names::FULLSCREEN, settings["fullscreen"]);
        m_gameSettings->SetValue(Settings::Names::VSYNC, settings["vsync"]);
        m_gameSettings->SetValue(Settings::Names::FRAME_RATE_LIMIT, settings["frameRateLimit"]);
        m_gameSettings->SetValue(Settings::Names::MASTER_VOLUME, settings["masterVolume"]);
        m_gameSettings->SetValue(Settings::Names::MUSIC_VOLUME, settings["musicVolume"]);
        m_gameSettings->SetValue(Settings::Names::SFX_VOLUME, settings["sfxVolume"]);
        m_gameSettings->SetValue(Settings::Names::CAMERA_ZOOM_SPEED, settings["cameraZoomSpeed"]);
        m_gameSettings->SetValue(Settings::Names::CAMERA_PAN_SPEED, settings["cameraPanSpeed"]);
        m_gameSettings->SetValue(Settings::Names::AUTOSAVE_INTERVAL, settings["autosaveInterval"]);

        return true;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error deserializing game state: ", e.what());
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