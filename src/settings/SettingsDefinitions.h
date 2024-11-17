#pragma once

#include <SFML/Graphics.hpp>

namespace Settings {
    namespace Names {
        // Video Settings
        constexpr const char* RESOLUTION = "video.resolution";
        constexpr const char* FULLSCREEN = "video.fullscreen";
        constexpr const char* VSYNC = "video.vsync";
        constexpr const char* FRAME_RATE_LIMIT = "video.frameRateLimit";

        // Audio Settings
        constexpr const char* MASTER_VOLUME = "audio.masterVolume";
        constexpr const char* MUSIC_VOLUME = "audio.musicVolume";
        constexpr const char* SFX_VOLUME = "audio.sfxVolume";

        // Gameplay Settings
        constexpr const char* CAMERA_ZOOM_SPEED = "gameplay.cameraZoomSpeed";
        constexpr const char* CAMERA_PAN_SPEED = "gameplay.cameraPanSpeed";
        constexpr const char* AUTOSAVE_INTERVAL = "gameplay.autosaveInterval";
    }

    namespace Categories {
        constexpr const char* VIDEO = "Video";
        constexpr const char* AUDIO = "Audio";
        constexpr const char* GAMEPLAY = "Gameplay";
    }

    namespace Defaults {
        const sf::Vector2u RESOLUTION(1920, 1080);
        constexpr bool FULLSCREEN = false;
        constexpr bool VSYNC = true;
        constexpr unsigned int FRAME_RATE_LIMIT = 60;

        constexpr float MASTER_VOLUME = 1.0f;
        constexpr float MUSIC_VOLUME = 0.8f;
        constexpr float SFX_VOLUME = 0.8f;

        constexpr float CAMERA_ZOOM_SPEED = 1.1f;
        constexpr float CAMERA_PAN_SPEED = 500.0f;
        constexpr unsigned int AUTOSAVE_INTERVAL = 5;
    }
} 