#include "MapLoader.h"
#include "OSMDataStore.h"
#include "PBFParser.h"
#include "ProgressTracker.h"
#include "../Debug.h"

MapLoader::MapLoader(
    std::shared_ptr<ThreadManager> threadManager,
    std::shared_ptr<StateManager> stateManager,
    std::shared_ptr<ResourceManager> resourceManager
)
    : m_threadManager(threadManager)
    , m_stateManager(stateManager)
    , m_resourceManager(resourceManager)
    , m_dataStore(std::make_shared<OSMDataStore>())
{}

std::future<MapLoader::LoadStatus> MapLoader::LoadMapAsync(
    const std::string& filePath,
    const LoaderConfig& config,
    std::function<void(float)> progressCallback
) {
    // Check if already loading
    if (m_isLoading) {
        m_lastError = "Map loading already in progress";
        auto promise = std::promise<LoadStatus>();
        promise.set_value(LoadStatus::Unknown);
        return promise.get_future();
    }

    // Create promise for the result
    auto promise = std::make_shared<std::promise<LoadStatus>>();
    auto future = promise->get_future();

    try {
        // Create progress tracker if callback provided
        if (progressCallback) {
            m_progressTracker = std::make_unique<ProgressTracker>(progressCallback);
        }

        // Set loading state
        m_isLoading = true;
        m_stateManager->SetState(STATE_LOADING, true);

        // Create task for asynchronous execution
        m_threadManager->EnqueueTask(
            "MapLoading",
            ThreadPriority::High,
            [this, promise, filePath, config]() {
                try {
                    // Execute loading operation
                    LoadStatus result = ExecuteLoading(filePath, config);

                    // Reset loading state
                    m_isLoading = false;
                    m_stateManager->SetState(STATE_LOADING, false);

                    // Clear progress tracker
                    m_progressTracker.reset();

                    // Set the promise value
                    promise->set_value(result);
                }
                catch (const std::exception& e) {
                    // Handle any unexpected exceptions
                    m_lastError = std::string("Unexpected error during map loading: ") + e.what();
                    m_isLoading = false;
                    m_stateManager->SetState(STATE_LOADING, false);
                    m_progressTracker.reset();
                    promise->set_value(LoadStatus::Unknown);
                }
            }
        );

        return future;
    }
    catch (const std::exception& e) {
        // Handle any exceptions during task creation
        m_lastError = std::string("Failed to start asynchronous loading: ") + e.what();
        m_isLoading = false;
        m_stateManager->SetState(STATE_LOADING, false);
        m_progressTracker.reset();
        promise->set_value(LoadStatus::Unknown);
        return future;
    }
}

MapLoader::LoadStatus MapLoader::LoadMap(
    const std::string& filePath,
    const LoaderConfig& config,
    std::function<void(float)> progressCallback
) {
    // Check if already loading
    if (m_isLoading) {
        m_lastError = "Map loading already in progress";
        return LoadStatus::Unknown;
    }

    try {
        // Create progress tracker if callback provided
        if (progressCallback) {
            m_progressTracker = std::make_unique<ProgressTracker>(progressCallback);
        }

        // Set loading state
        m_isLoading = true;
        m_stateManager->SetState(STATE_LOADING, true);

        // Execute loading operation
        LoadStatus result = ExecuteLoading(filePath, config);

        // Reset loading state
        m_isLoading = false;
        m_stateManager->SetState(STATE_LOADING, false);

        // Clear progress tracker
        m_progressTracker.reset();

        return result;
    }
    catch (const std::exception& e) {
        // Handle any unexpected exceptions
        m_lastError = std::string("Unexpected error during map loading: ") + e.what();
        m_isLoading = false;
        m_stateManager->SetState(STATE_LOADING, false);
        m_progressTracker.reset();
        return LoadStatus::Unknown;
    }
}

void MapLoader::CancelLoading() {
    if (!m_isLoading) {
        return;
    }

    // Set the cancel flag
    m_cancelRequested = true;

    // Signal the parser to cancel if it exists
    if (m_parser) {
        m_parser->SetCancelled(true);
    }

    // Update the state manager to reflect cancellation
    m_stateManager->SetState(STATE_LOADING, false);
    
    // Wait for loading to complete
    while (m_isLoading) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Reset cancel flag after loading has stopped
    m_cancelRequested = false;
}

bool MapLoader::IsLoading() const {
    return m_isLoading;
}

std::optional<std::string> MapLoader::GetLastError() const {
    return m_lastError;
}

std::shared_ptr<const OSMDataStore> MapLoader::GetMapData() const {
    return m_dataStore;
}

MapLoader::LoadStatus MapLoader::ExecuteLoading(
    const std::string& filePath,
    const LoaderConfig& config
) {
    // Reset state
    m_cancelRequested = false;
    m_lastError.reset();

    try {
        // Create new data store
        m_dataStore = std::make_shared<OSMDataStore>();

        // Validate file exists and is readable
        if (!std::filesystem::exists(filePath)) {
            m_lastError = "File not found: " + filePath;
            return LoadStatus::FileNotFound;
        }

        // Get file size for progress tracking
        size_t fileSize = std::filesystem::file_size(filePath);
        if (m_progressTracker) {
            m_progressTracker->UpdateBytes(0, fileSize);
            m_progressTracker->SetStatus("Opening map file...");
        }

        // Create file mapping for efficient reading
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            m_lastError = "Failed to open file: " + filePath;
            return LoadStatus::FileNotFound;
        }

        // Create PBF parser with configuration
        m_parser = std::make_unique<PBFParser>(m_dataStore);
        if (m_progressTracker) {
            m_parser->SetProgressTracker(m_progressTracker);
        }

        // Configure parser
        PBFParser::ParserConfig parserConfig;
        parserConfig.parseNodes = config.loadBuildings || config.loadRoads;
        parserConfig.parseWays = config.loadRoads || config.loadWaterways;
        parserConfig.parseRelations = config.loadLandUse;
        parserConfig.blockSize = config.chunkSize;

        // Process file in chunks
        std::vector<uint8_t> buffer(config.chunkSize);
        size_t totalBytesRead = 0;

        while (file && !m_cancelRequested) {
            // Read chunk
            file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            std::streamsize bytesRead = file.gcount();
            
            if (bytesRead <= 0) {
                break;  // End of file or error
            }

            // Resize buffer to actual bytes read
            buffer.resize(static_cast<size_t>(bytesRead));

            // Process chunk through parser
            m_parser->ProcessBlock(buffer, parserConfig);
            
            // Check for parser errors
            if (m_parser->HasError()) {
                m_lastError = m_parser->GetLastError();
                return LoadStatus::InvalidFormat;
            }

            // Update progress
            totalBytesRead += static_cast<size_t>(bytesRead);
            if (m_progressTracker) {
                m_progressTracker->UpdateBytes(totalBytesRead, fileSize);
                m_progressTracker->SetStatus("Processing map data... " + 
                    std::to_string(totalBytesRead / (1024 * 1024)) + "MB processed");
            }

            // Prepare buffer for next chunk
            buffer.resize(config.chunkSize);
        }

        // Check if operation was cancelled
        if (m_cancelRequested) {
            m_lastError = "Operation cancelled by user";
            return LoadStatus::Cancelled;
        }

        // Clean up resources
        file.close();
        m_parser.reset();

        // Final progress update
        if (m_progressTracker) {
            m_progressTracker->UpdateProgress(1.0f);
            m_progressTracker->SetStatus("Map loading complete");
        }

        return LoadStatus::Success;
    }
    catch (const std::bad_alloc& e) {
        m_lastError = std::string("Memory allocation failed: ") + e.what();
        return LoadStatus::MemoryError;
    }
    catch (const std::exception& e) {
        m_lastError = std::string("Unexpected error: ") + e.what();
        return LoadStatus::Unknown;
    }
} 