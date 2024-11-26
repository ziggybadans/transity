#pragma once

#include <string>
#include <memory>
#include <functional>
#include <future>
#include <optional>
#include "../utility/ThreadManager.h"
#include "../core/StateManager.h"
#include "../managers/ResourceManager.h"

/* Forward declarations */
class OSMDataStore;
class PBFParser;
class ProgressTracker;
class FileMapping;

/**
 * @brief Class responsible for coordinating the loading of OpenStreetMap data
 * Acts as a facade for the map loading subsystem
 */
class MapLoader {
public:
    // Represents the loading status
    enum class LoadStatus {
        Success,
        FileNotFound,
        InvalidFormat,
        MemoryError,
        Cancelled,
        Unknown
    };

    // Configuration for the loader
    struct LoaderConfig {
        bool loadBuildings{true};
        bool loadRoads{true};
        bool loadWaterways{true};
        bool loadLandUse{true};
        size_t maxThreads{4};
        size_t chunkSize{32 * 1024 * 1024}; // 32MB chunks
    };

    MapLoader(
        std::shared_ptr<ThreadManager> threadManager,
        std::shared_ptr<StateManager> stateManager,
        std::shared_ptr<ResourceManager> resourceManager
    );
    ~MapLoader() = default;

    /**
     * @brief Asynchronously loads an OSM PBF file
     * @param filePath Path to the .osm.pbf file
     * @param config Loader configuration
     * @param progressCallback Callback for loading progress (0.0 to 1.0)
     * @return Future containing the load status
     */
    std::future<LoadStatus> LoadMapAsync(
        const std::string& filePath,
        const LoaderConfig& config = LoaderConfig{},
        std::function<void(float)> progressCallback = nullptr
    );

    /**
     * @brief Synchronously loads an OSM PBF file
     * @param filePath Path to the .osm.pbf file
     * @param config Loader configuration
     * @param progressCallback Callback for loading progress (0.0 to 1.0)
     * @return Load status
     */
    LoadStatus LoadMap(
        const std::string& filePath,
        const LoaderConfig& config = LoaderConfig{},
        std::function<void(float)> progressCallback = nullptr
    );

    // Cancels the current loading operation
    void CancelLoading();

    // Returns true if currently loading a map
    bool IsLoading() const;

    // Returns the last loading error message if any
    std::optional<std::string> GetLastError() const;
    std::shared_ptr<const OSMDataStore> GetMapData() const;

private:
    LoadStatus ExecuteLoading(const std::string& filePath, const LoaderConfig& config);

    std::shared_ptr<ThreadManager> m_threadManager;
    std::shared_ptr<StateManager> m_stateManager;
    std::shared_ptr<ResourceManager> m_resourceManager;
    
    std::unique_ptr<PBFParser> m_parser;
    std::unique_ptr<ProgressTracker> m_progressTracker;
    std::shared_ptr<OSMDataStore> m_dataStore;
    
    bool m_isLoading{false};
    bool m_cancelRequested{false};
    std::optional<std::string> m_lastError;

    static constexpr const char* STATE_LOADING = "map_loading";
    static constexpr const char* STATE_PROGRESS = "map_progress";
};
