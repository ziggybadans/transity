#include "GameSettings.h"
#include <fstream>
#include <iostream>
#include "../Debug.h"
#include <filesystem>

GameSettings::GameSettings()
    : m_resolution(1920, 1080)
    , m_fullscreen(false)
    , m_vsyncEnabled(true)
    , m_frameRateLimit(144)
    , m_masterVolume(1.0f)
    , m_musicVolume(0.8f)
    , m_sfxVolume(0.8f)
    , m_cameraZoomSpeed(1.1f)
    , m_cameraPanSpeed(500.0f)
    , m_autosaveInterval(5)
{}

bool GameSettings::LoadSettings(const std::string& filepath) {
    try {
        // Create config directory if it doesn't exist
        std::filesystem::path configPath = std::filesystem::path(filepath).parent_path();
        std::filesystem::create_directories(configPath);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            DEBUG_ERROR("Failed to open settings file: ", filepath);
            return false;
        }

        nlohmann::json j;
        file >> j;

        // Video settings
        auto res = j["video"]["resolution"];
        m_resolution = sf::Vector2u(res[0], res[1]);
        m_fullscreen = j["video"]["fullscreen"];
        m_vsyncEnabled = j["video"]["vsync"];
        m_frameRateLimit = j["video"]["frameRateLimit"];

        // Audio settings
        m_masterVolume = j["audio"]["masterVolume"];
        m_musicVolume = j["audio"]["musicVolume"];
        m_sfxVolume = j["audio"]["sfxVolume"];

        // Gameplay settings
        m_cameraZoomSpeed = j["gameplay"]["cameraZoomSpeed"];
        m_cameraPanSpeed = j["gameplay"]["cameraPanSpeed"];
        m_autosaveInterval = j["gameplay"]["autosaveInterval"];

        return true;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error loading settings: ", e.what());
        return false;
    }
}

bool GameSettings::SaveSettings(const std::string& filepath) const {
    try {
        // Create config directory if it doesn't exist
        std::filesystem::path configPath = std::filesystem::path(filepath).parent_path();
        std::filesystem::create_directories(configPath);

        nlohmann::json j;

        // Video settings
        j["video"]["resolution"] = {m_resolution.x, m_resolution.y};
        j["video"]["fullscreen"] = m_fullscreen;
        j["video"]["vsync"] = m_vsyncEnabled;
        j["video"]["frameRateLimit"] = m_frameRateLimit;

        // Audio settings
        j["audio"]["masterVolume"] = m_masterVolume;
        j["audio"]["musicVolume"] = m_musicVolume;
        j["audio"]["sfxVolume"] = m_sfxVolume;

        // Gameplay settings
        j["gameplay"]["cameraZoomSpeed"] = m_cameraZoomSpeed;
        j["gameplay"]["cameraPanSpeed"] = m_cameraPanSpeed;
        j["gameplay"]["autosaveInterval"] = m_autosaveInterval;

        std::ofstream file(filepath);
        file << j.dump(4);
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error saving settings: ", e.what());
        return false;
    }
}

// Setter implementations
void GameSettings::SetResolution(const sf::Vector2u& resolution) {
    m_resolution = resolution;
}

void GameSettings::SetFullscreen(bool fullscreen) {
    m_fullscreen = fullscreen;
}

void GameSettings::SetVSync(bool enabled) {
    m_vsyncEnabled = enabled;
}

void GameSettings::SetFrameRateLimit(unsigned int limit) {
    m_frameRateLimit = limit;
}

void GameSettings::SetMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

void GameSettings::SetMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 1.0f);
}

void GameSettings::SetSFXVolume(float volume) {
    m_sfxVolume = std::clamp(volume, 0.0f, 1.0f);
}

void GameSettings::SetCameraZoomSpeed(float speed) {
    m_cameraZoomSpeed = std::max(1.0f, speed);
}

void GameSettings::SetCameraPanSpeed(float speed) {
    m_cameraPanSpeed = std::max(0.0f, speed);
}

void GameSettings::SetAutosaveInterval(unsigned int minutes) {
    m_autosaveInterval = minutes;
} 