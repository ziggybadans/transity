#pragma once

namespace Constants {
    // Window Settings
    constexpr unsigned int WINDOW_WIDTH = 3440;
    constexpr unsigned int WINDOW_HEIGHT = 1440;
    constexpr const char* WINDOW_TITLE = "Transity";

    // Camera Settings
    constexpr float CAMERA_ZOOM_SPEED = 1.1f;
    constexpr float CAMERA_PAN_SPEED = 2.0f;
    constexpr float CAMERA_MIN_ZOOM = 0.1f;
    constexpr float CAMERA_MAX_ZOOM = 1.0f;

    // World Settings
    constexpr unsigned int MAP_SIZE = 25;
    constexpr float TILE_SIZE = 32.0f;
}
