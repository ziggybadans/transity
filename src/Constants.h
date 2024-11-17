#pragma once

namespace Constants {
    // Window Settings
    constexpr unsigned int WINDOW_WIDTH = 3440;
    constexpr unsigned int WINDOW_HEIGHT = 1440;
    constexpr const char* WINDOW_TITLE = "OSM World Viewer";

    // World Settings
    constexpr float WORLD_WIDTH = 3600.0f;  // 360 degrees * 10 for higher precision
    constexpr float WORLD_HEIGHT = 1800.0f; // 180 degrees * 10 for higher precision

    // Camera Settings
    constexpr float CAMERA_ZOOM_SPEED = 1.1f;
    constexpr float CAMERA_PAN_SPEED = 500.0f;
    constexpr float CAMERA_MIN_ZOOM = 0.001f;
    constexpr float CAMERA_MAX_ZOOM = 1.0f;
}
