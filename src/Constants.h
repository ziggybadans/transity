#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#include <string_view>

#ifndef TRANSITY_VERSION
#define TRANSITY_VERSION "0.1.0"
#endif

#ifndef TRANSITY_STAGE
#define TRANSITY_STAGE "Predev"
#endif

namespace Constants {
    constexpr float EPSILON = 0.001f;

    //=========================================================================
    // App
    //=========================================================================
    inline constexpr std::string_view APP_NAME = "Transity";
    inline constexpr std::string_view APP_STAGE = TRANSITY_STAGE;
    inline constexpr std::string_view APP_VERSION = TRANSITY_VERSION;
    inline constexpr std::string_view APP_COPYRIGHT = "Copyright 2025 Ziggy Badans. MIT License.";
    constexpr int SUPPORTED_ARCHETYPE_VERSION = 1;

    inline const std::string &windowTitle() {
        static const std::string title = [] {
            std::string value(APP_NAME);
            if (!APP_STAGE.empty()) {
                value.append(" - ");
                value.append(APP_STAGE.data(), APP_STAGE.size());
            }
            return value;
        }();
        return title;
    }

    inline const std::string &versionBanner() {
        static const std::string banner = [] {
            std::string value(APP_NAME);
            if (!APP_STAGE.empty()) {
                value.push_back(' ');
                value.append(APP_STAGE.data(), APP_STAGE.size());
            }
            if (!APP_VERSION.empty()) {
                value.push_back(' ');
                value.push_back('v');
                value.append(APP_VERSION.data(), APP_VERSION.size());
            }
            return value;
        }();
        return banner;
    }

    inline const std::string &copyrightNotice() {
        static const std::string notice(APP_COPYRIGHT);
        return notice;
    }

    //=========================================================================
    // Graphics & UI
    //=========================================================================
    constexpr unsigned int WINDOW_WIDTH = 3840;
    constexpr unsigned int WINDOW_HEIGHT = 2160;
    constexpr unsigned int FRAMERATE_LIMIT = 144;
    constexpr float UI_WINDOW_PADDING = 10.0f;
    constexpr float UI_WORLD_GEN_SETTINGS_WIDTH = 400.0f;
    constexpr float UI_LINE_CREATION_WINDOW_WIDTH = 210.0f;
    constexpr float UI_INTERACTION_MODES_WIDTH = 200.0f;

    //=========================================================================
    // Camera
    //=========================================================================
    constexpr float DYNAMIC_CAMERA_SPEED_MULTIPLIER = 0.5f;
    constexpr float ZOOM_FACTOR = 0.9f;
    constexpr float UNZOOM_FACTOR = 1.0f / ZOOM_FACTOR;
    constexpr float CAMERA_MIN_ZOOM = 0.03f;
    constexpr float CAMERA_MAX_ZOOM = 1.0f;

    //=========================================================================
    // World Generation
    //=========================================================================
    // --- City Placement ---
    constexpr int INITIAL_CITY_COUNT = 4;
    constexpr int MAX_CITIES = 50;
    constexpr float MIN_CITY_SPAWN_INTERVAL_S = 15.0f;
    constexpr float MAX_CITY_SPAWN_INTERVAL_S = 180.0f;
    constexpr float CITY_PLACEMENT_NOISE_FREQUENCY = 0.005f;

    // --- City Suitability ---
    constexpr float SUITABILITY_WEIGHT_WATER = 0.20f;
    constexpr float SUITABILITY_WEIGHT_EXPANDABILITY = 0.25f;
    constexpr float SUITABILITY_WEIGHT_PROXIMITY = 0.35f;
    constexpr float SUITABILITY_WEIGHT_RANDOMNESS = 0.20f;
    constexpr float CITY_PROXIMITY_IDEAL_DISTANCE = 80.0f;
    constexpr int WATER_SUITABILITY_MAX_DISTANCE = 60;
    constexpr int EXPANDABILITY_SUITABILITY_RADIUS = 20;
    constexpr float SUBURB_PROXIMITY_RANGE_CAPITAL = 100.0f;
    constexpr float SUBURB_PROXIMITY_RANGE_TOWN = 50.0f;
    constexpr float TOWN_PROXIMITY_MIN_DISTANCE = 50.0f;
    constexpr float TOWN_PROXIMITY_MAX_DISTANCE = 150.0f;

    // --- City Finding Algorithm ---
    constexpr int FIND_BEST_CITY_LOCATION_SAMPLES = 5000;
    constexpr int FIND_BEST_CITY_LOCATION_TOP_CANDIDATES = 50;
    constexpr int FIND_RANDOM_CITY_LOCATION_ATTEMPTS = 100;
    constexpr float FIND_RANDOM_CITY_MIN_SUITABILITY = 0.4f;
    constexpr float FIND_RANDOM_CITY_MAX_SUITABILITY = 0.7f;

    //=========================================================================
    // Gameplay
    //=========================================================================
    // --- Line ---
    constexpr float DEFAULT_LINE_THICKNESS = 5.0f;
    constexpr float LINE_SNAP_RADIUS = 20.0f;
    constexpr float LINE_CENTER_SNAP_RADIUS = 4.0f;
    constexpr float LINE_PARALLEL_OFFSET = 12.0f;
    constexpr float METRO_CURVE_RADIUS = 200.0f;

    // --- Train ---
    constexpr float TRAIN_STOP_DURATION = 2.0f;
    constexpr float TRAIN_MAX_SPEED = 100.0f;
    constexpr float TRAIN_ACCELERATION = 25.0f;
    constexpr int TRAIN_CAPACITY = 20;

    // --- Passenger ---
    constexpr float PASSENGER_SPAWN_ANIMATION_DURATION = 1.0f;

}
