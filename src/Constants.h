#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Constants {
    constexpr float EPSILON = 0.001f;

    // Versioning
    constexpr const char *WINDOW_TITLE = "Transity Predev";
    constexpr int SUPPORTED_ARCHETYPE_VERSION = 1;
    
    // Logging
    constexpr unsigned int TRACE_LOG_DELAY_MS = 2000;

    // Graphics
    constexpr unsigned int WINDOW_WIDTH = 1920;
    constexpr unsigned int WINDOW_HEIGHT = 1080;
    constexpr unsigned int FRAMERATE_LIMIT = 144;
    constexpr unsigned int LINE_VERTEX_COUNT = 2;

    // Camera
    constexpr float CAMERA_SPEED = 200.0f;
    constexpr float ZOOM_FACTOR = 0.9f;
    constexpr float UNZOOM_FACTOR = 1.0f / ZOOM_FACTOR;
    constexpr float DYNAMIC_CAMERA_SPEED_MULTIPLIER = 0.5f;
    constexpr float CAMERA_MIN_ZOOM = 0.03f;  // Most zoomed out
    constexpr float CAMERA_MAX_ZOOM = 1.0f;   // Most zoomed in

    // World Generation
    constexpr int ISLAND_BASE_SHAPE_POINTS = 8;
    constexpr float COASTLINE_DISTORTION_FREQUENCY = 0.05f;
    constexpr float COASTLINE_DISTORTION_STRENGTH = 15.0f;
    constexpr float COASTLINE_SUBDIVISION_LENGTH = 20.0f;
    constexpr int NUM_CITIES_TO_GENERATE = 10;

    // UI
    constexpr float UI_WINDOW_PADDING = 10.0f;
    constexpr float UI_WORLD_GEN_SETTINGS_WIDTH = 300.0f;
    constexpr float UI_LINE_CREATION_WINDOW_WIDTH = 210.0f;
    constexpr float UI_INTERACTION_MODES_WIDTH = 200.0f;
    constexpr float UI_INTERACTION_MODES_HEIGHT = 100.0f;

    // City Placement
    constexpr int INITIAL_CITY_COUNT = 5;
    constexpr int MAX_CITIES = 50;
    constexpr float MIN_CITY_SPAWN_INTERVAL_S = 15.0f;
    constexpr float MAX_CITY_SPAWN_INTERVAL_S = 180.0f;
    constexpr float CITY_PLACEMENT_NOISE_FREQUENCY = 0.005f;

    // City Suitability
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

    // City Finding Algorithm
    constexpr int FIND_BEST_CITY_LOCATION_SAMPLES = 5000;
    constexpr int FIND_BEST_CITY_LOCATION_TOP_CANDIDATES = 50;
    constexpr int FIND_RANDOM_CITY_LOCATION_ATTEMPTS = 100;
    constexpr float FIND_RANDOM_CITY_MIN_SUITABILITY = 0.4f;
    constexpr float FIND_RANDOM_CITY_MAX_SUITABILITY = 0.7f;

    // Line
    constexpr float DEFAULT_LINE_THICKNESS = 5.0f;
    const float LINE_SNAP_RADIUS = 20.0f;
    const float LINE_CENTER_SNAP_RADIUS = 4.0f;
    const float LINE_PARALLEL_OFFSET = 12.0f;
    constexpr float METRO_CURVE_RADIUS = 100.0f;

    // Train
    constexpr float TRAIN_DEFAULT_INITIAL_SPEED = 50.0f;
    constexpr float TRAIN_STOP_DURATION = 2.0f;
    constexpr float TRAIN_MIN_SPEED = 5.0f;
    constexpr float TRAIN_MAX_SPEED = 100.0f;
    constexpr float TRAIN_ACCELERATION = 25.0f;
    constexpr int TRAIN_CAPACITY = 20;

    // Passenger
    constexpr float PASSENGER_SPAWN_ANIMATION_DURATION = 1.0f;
}  // namespace Constants
