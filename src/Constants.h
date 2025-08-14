#pragma once

namespace Constants {
constexpr unsigned int WINDOW_WIDTH = 1920;
constexpr unsigned int WINDOW_HEIGHT = 1080;
constexpr const char *WINDOW_TITLE = "Transity Predev";

constexpr unsigned char CLEAR_COLOR_R = 173;
constexpr unsigned char CLEAR_COLOR_G = 216;
constexpr unsigned char CLEAR_COLOR_B = 230;

constexpr unsigned int FRAMERATE_LIMIT = 144;
constexpr unsigned int TRACE_LOG_DELAY_MS = 2000;
constexpr unsigned int LINE_VERTEX_COUNT = 2;

constexpr float CAMERA_SPEED = 200.0f;
constexpr float ZOOM_FACTOR = 0.9f;
constexpr float UNZOOM_FACTOR = 1.0f / ZOOM_FACTOR;
constexpr float DYNAMIC_CAMERA_SPEED_MULTIPLIER = 0.5f;
constexpr float CAMERA_MIN_ZOOM = 0.03f;  // Most zoomed out
constexpr float CAMERA_MAX_ZOOM = 1.0f;   // Most zoomed in

constexpr int ISLAND_BASE_SHAPE_POINTS = 8;
constexpr float COASTLINE_DISTORTION_FREQUENCY = 0.05f;
constexpr float COASTLINE_DISTORTION_STRENGTH = 15.0f;
constexpr float COASTLINE_SUBDIVISION_LENGTH = 20.0f;

constexpr float UI_WINDOW_PADDING = 10.0f;
constexpr float UI_WORLD_GEN_SETTINGS_WIDTH = 300.0f;
constexpr float UI_INTERACTION_MODES_WIDTH = 400.0f;
constexpr float UI_INTERACTION_MODES_HEIGHT = 100.0f;
}  // namespace Constants
