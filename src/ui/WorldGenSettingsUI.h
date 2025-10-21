#pragma once

#include "components/WorldComponents.h"
#include "event/EventBus.h"
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

struct MouseMovedEvent;

class WorldGenerationSystem;
class TerrainRenderSystem;

class WorldGenSettingsUI {
public:
    WorldGenSettingsUI(EventBus &eventBus, WorldGenerationSystem &worldGenerationSystem,
                       TerrainRenderSystem &terrainRenderSystem);
    ~WorldGenSettingsUI();

    void draw();

private:
    void drawNoiseLayerSettings(WorldGenParams &params, bool &paramsChanged);
    void drawWorldGridSettings(WorldGenParams &params, bool &gridChanged);
    void drawElevationSettings(WorldGenParams &params, bool &paramsChanged);
    void drawVisualizationSettings();
    void drawActions(const WorldGenParams &params);
    bool drawResetButton(const char *label);
    bool sliderFloatWithReset(const char *label, float *value, float defaultValue, float min,
                              float max, const char *format = "%.3f");
    bool sliderIntWithReset(const char *label, int *value, int defaultValue, int min, int max,
                            const char *format = "%d");
    void onMouseMoved(const MouseMovedEvent &event);

    EventBus &_eventBus;
    WorldGenerationSystem &_worldGenerationSystem;
    TerrainRenderSystem &_terrainRenderSystem;
    WorldGenParams _defaultParams;

    bool _autoRegenerate = false;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _visualizeSuitabilityMap = false;
    int _selectedSuitabilityMap = 4;
    bool _shadedReliefEnabled = false;
    bool _hasMouseWorldPos = false;
    sf::Vector2f _lastMouseWorldPos{0.f, 0.f};
    entt::scoped_connection _mouseMovedConnection;
};
