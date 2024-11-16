#pragma once

namespace Constants {
    // Window Settings
    constexpr unsigned int WINDOW_WIDTH = 3440;
    constexpr unsigned int WINDOW_HEIGHT = 1440;
    constexpr const char* WINDOW_TITLE = "2D Transport Management Game";

    // Camera Settings
    constexpr float CAMERA_ZOOM_SPEED = 1.1f;     // 10% zoom per scroll
    constexpr float CAMERA_PAN_SPEED = 2000.0f;   // 2000 units per second
    constexpr float CAMERA_MIN_ZOOM = 0.001f;      // Minimum zoom level
    constexpr float CAMERA_MAX_ZOOM = 1.0f;        // Maximum zoom level

    static constexpr float kmPerUnit = 11.1f; // Approximate km per game unit
}
