#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>

class GameSettings {
public:
    GameSettings();
    ~GameSettings() = default;

    /* Core Methods */
    bool LoadSettings(const std::string& filepath);
    bool SaveSettings(const std::string& filepath) const;

    /* Video Settings */
    void SetResolution(const sf::Vector2u& resolution);
    void SetFullscreen(bool fullscreen);
    void SetVSync(bool enabled);
    void SetFrameRateLimit(unsigned int limit);

    /* Audio Settings */
    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetSFXVolume(float volume);

    /* Gameplay Settings */
    void SetCameraZoomSpeed(float speed);
    void SetCameraPanSpeed(float speed);
    void SetAutosaveInterval(unsigned int minutes);

    /* Getters */
    const sf::Vector2u& GetResolution() const { return m_resolution; }
    bool IsFullscreen() const { return m_fullscreen; }
    bool IsVSyncEnabled() const { return m_vsyncEnabled; }
    unsigned int GetFrameRateLimit() const { return m_frameRateLimit; }
    float GetMasterVolume() const { return m_masterVolume; }
    float GetMusicVolume() const { return m_musicVolume; }
    float GetSFXVolume() const { return m_sfxVolume; }
    float GetCameraZoomSpeed() const { return m_cameraZoomSpeed; }
    float GetCameraPanSpeed() const { return m_cameraPanSpeed; }
    unsigned int GetAutosaveInterval() const { return m_autosaveInterval; }

private:
    /* Video Settings */
    sf::Vector2u m_resolution;
    bool m_fullscreen;
    bool m_vsyncEnabled;
    unsigned int m_frameRateLimit;

    /* Audio Settings */
    float m_masterVolume;
    float m_musicVolume;
    float m_sfxVolume;

    /* Gameplay Settings */
    float m_cameraZoomSpeed;
    float m_cameraPanSpeed;
    unsigned int m_autosaveInterval;
}; 