#pragma once

#include "components/WorldComponents.h"
#include "event/EventBus.h"

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
    void drawVisualizationSettings();
    void drawActions(const WorldGenParams &params);

    EventBus &_eventBus;
    WorldGenerationSystem &_worldGenerationSystem;
    TerrainRenderSystem &_terrainRenderSystem;

    bool _autoRegenerate = false;
    bool _visualizeChunkBorders = false;
    bool _visualizeCellBorders = false;
    bool _visualizeSuitabilityMap = false;
    int _selectedSuitabilityMap = 4;
    bool _isLodEnabled = false;
};