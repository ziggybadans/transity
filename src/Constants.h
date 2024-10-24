#pragma once

namespace Constants {
    // Window Settings
    constexpr unsigned int WINDOW_WIDTH = 3440;
    constexpr unsigned int WINDOW_HEIGHT = 1440;
    constexpr const char* WINDOW_TITLE = "2D Transport Management Game";

    // Camera Settings
    constexpr float CAMERA_ZOOM_SPEED = 1.1f;     // 10% zoom per scroll
    constexpr float CAMERA_PAN_SPEED = 1000.0f;   // 1000 units per second
    constexpr float CAMERA_MIN_ZOOM = 0.01f;      // Minimum zoom level
    constexpr float CAMERA_MAX_ZOOM = 5.0f;       // Maximum zoom level

    // World Map Settings
    constexpr const char* HIGH_RES_MAP_PATH = "assets/world_high_detail.png";
    constexpr const char* LOW_RES_MAP_PATH = "assets/world_low_detail.png";
    constexpr float WORLD_MAP_ZOOM_SWITCH = 1.0f; // Zoom level to switch textures
}
