#include "WorldGenerationSystem.h"
#include "../Logger.h"
#include "../core/Constants.h"

#include <random>
#include <algorithm>
#include <vector>

WorldGenerationSystem::WorldGenerationSystem(entt::registry& registry, EventBus& eventBus)
    : _registry(registry), _eventBus(eventBus) { // <-- MODIFIED
    // Connect to the event bus using the member variable
    _regenerateWorldListener = _eventBus.sink<RegenerateWorldRequestEvent>().connect<&WorldGenerationSystem::onRegenerateWorldRequest>(this);
    _worldGenParamsListener = _eventBus.sink<WorldGenParamsChangeEvent>().connect<&WorldGenerationSystem::onWorldGenParamsChange>(this);
    LOG_INFO("WorldGenerationSystem", "System created and listening for world generation events.");
}

WorldGenerationSystem::~WorldGenerationSystem() {
    // Disconnect using the member variable
    _eventBus.sink<RegenerateWorldRequestEvent>().disconnect(this);
    _eventBus.sink<WorldGenParamsChangeEvent>().disconnect(this);
}

void WorldGenerationSystem::setParams(const WorldGenParams& params) {
    _params = params;
    configureNoise();
}

void WorldGenerationSystem::configureNoise() {
    _noiseGenerator.SetSeed(_params.seed);
    _noiseGenerator.SetFrequency(_params.frequency);
    _noiseGenerator.SetNoiseType(_params.noiseType);
    _noiseGenerator.SetFractalType(_params.fractalType);
    _noiseGenerator.SetFractalOctaves(_params.octaves);
    _noiseGenerator.SetFractalLacunarity(_params.lacunarity);
    _noiseGenerator.SetFractalGain(_params.gain);

    generateWorld(3, 3);
}

const WorldGridComponent& WorldGenerationSystem::getWorldGridSettings() {
    auto view = _registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("No WorldGridComponent found in the registry.");
    }
    return view.get<WorldGridComponent>(view.front());
}

sf::Vector2f WorldGenerationSystem::getWorldSize() {
    try {
        const WorldGridComponent& worldGrid = getWorldGridSettings();
        float worldWidth = static_cast<float>(worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x) * worldGrid.cellSize;
        float worldHeight = static_cast<float>(worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y) * worldGrid.cellSize;
        return {worldWidth, worldHeight};
    } catch (const std::runtime_error& e) {
        LOG_ERROR("WorldGenerationSystem", "Cannot get world size: %s", e.what());
        return {0.0f, 0.0f};
    }
}

bool isInside(const sf::Vector2f& point, const std::vector<sf::Vector2f>& polygon) {
    if (polygon.empty()) {
        return false;
    }
    int n = polygon.size();
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

void WorldGenerationSystem::generateChunk(entt::entity chunkEntity) {
    if (!_registry.all_of<ChunkComponent>(chunkEntity)) {
        LOG_ERROR("WorldGenerationSystem", "Error: Entity does not have a ChunkComponent for generation.");
        return;
    }

    ChunkComponent& chunk = _registry.get<ChunkComponent>(chunkEntity);
    const WorldGridComponent& worldGrid = getWorldGridSettings();

    float worldWidth = worldGrid.worldDimensionsInChunks.x 
        * worldGrid.chunkDimensionsInCells.x * worldGrid.cellSize;
    float worldHeight = worldGrid.worldDimensionsInChunks.y
        * worldGrid.chunkDimensionsInCells.y * worldGrid.cellSize;
    float worldCenterX = worldWidth / 2.0f;
    float worldCenterY = worldHeight / 2.0f;
    float maxWorldDimension = std::max(worldWidth, worldHeight);

    // Ensure cells vector is correctly sized (constructor should handle this)
    // chunk.cells.resize(worldGrid.chunkDimensionsInCells.x * worldGrid.chunkDimensionsInCells.y);

    LOG_INFO("WorldGenerationSystem", "Generating chunk at: (%d, %d)", chunk.chunkGridPosition.x, chunk.chunkGridPosition.y);

    for (int y = 0; y < worldGrid.chunkDimensionsInCells.y; ++y) {
        for (int x = 0; x < worldGrid.chunkDimensionsInCells.x; ++x) {
            float worldX = (chunk.chunkGridPosition.x * worldGrid.chunkDimensionsInCells.x + x) 
                * worldGrid.cellSize + (worldGrid.cellSize / 2.0f);
            float worldY = (chunk.chunkGridPosition.y * worldGrid.chunkDimensionsInCells.y + y) 
                * worldGrid.cellSize + (worldGrid.cellSize / 2.0f);

            int cellIndex = y * worldGrid.chunkDimensionsInCells.x + x;

            if (isInside({worldX, worldY}, _islandShape)) {
                // If the point is inside the island shape, we generate land
                chunk.cells[cellIndex] = TerrainType::LAND;
            } else {
                // Otherwise, we generate water
                chunk.cells[cellIndex] = TerrainType::WATER;
            }

            chunk.noiseValues[cellIndex] = 0.0f;
        }
    }
    // Optionally, mark the chunk as generated, e.g., by adding a GeneratedTag or setting a flag.
    // registry.emplace_or_replace<ChunkGeneratedTag>(chunkEntity);
    LOG_INFO("WorldGenerationSystem", "Chunk generation complete for: (%d, %d)", chunk.chunkGridPosition.x, chunk.chunkGridPosition.y);
}

void WorldGenerationSystem::generateWorld(int numChunksX, int numChunksY) {
    entt::entity worldGridEntity = entt::null;
    auto worldView = _registry.view<WorldGridComponent>();
    if (worldView.empty()) {
        worldGridEntity = _registry.create();
        _registry.emplace<WorldGridComponent>(worldGridEntity);
        LOG_INFO("WorldGenerationSystem", "Created WorldGridComponent entity.");
    } else {
        worldGridEntity = worldView.front();
    }

    auto& worldGrid = _registry.get<WorldGridComponent>(worldGridEntity);
    worldGrid.worldDimensionsInChunks = {numChunksX, numChunksY};

    std::vector<sf::Vector2f> baseShape = generateIslandBaseShape();
    if (_params.distortCoastline) {
        _islandShape = distortCoastline(baseShape);
    } else {
        _islandShape = baseShape;
    }

    LOG_INFO("WorldGenerationSystem", "Clearing existing chunk entities...");
    auto chunkView = _registry.view<ChunkComponent>();
    for (auto entity : chunkView) {
        _registry.destroy(entity);
    }
    LOG_INFO("WorldGenerationSystem", "Existing chunk entities cleared.");
    
    LOG_INFO("WorldGenerationSystem", "Generating world of %dx%d chunks.", worldGrid.worldDimensionsInChunks.x, worldGrid.worldDimensionsInChunks.y);
    for (int cy = 0; cy < worldGrid.worldDimensionsInChunks.y; ++cy) {
        for (int cx = 0; cx < worldGrid.worldDimensionsInChunks.x; ++cx) {
            entt::entity newChunkEntity = _registry.create();
            ChunkComponent& chunkComp = _registry.emplace<ChunkComponent>(newChunkEntity,
                worldGrid.chunkDimensionsInCells.x, worldGrid.chunkDimensionsInCells.y);
            chunkComp.chunkGridPosition = {cx, cy};
            // The ChunkComponent constructor already initializes cells to WATER.
            
            // Add PositionComponent for the chunk entity itself if needed for spatial queries or rendering chunk boundaries
            // registry.emplace<PositionComponent>(newChunkEntity, sf::Vector2f{cx * CHUNK_SIZE_X * worldGrid.cellSize, cy * CHUNK_SIZE_Y * worldGrid.cellSize});

            generateChunk(newChunkEntity);
        }
    }
    LOG_INFO("WorldGenerationSystem", "Initial world generation finished.");
}

std::vector<sf::Vector2f> WorldGenerationSystem::generateIslandBaseShape() {
    const auto& worldGrid = getWorldGridSettings();
    float worldWidth = worldGrid.worldDimensionsInChunks.x * worldGrid.chunkDimensionsInCells.x * worldGrid.cellSize;
    float worldHeight = worldGrid.worldDimensionsInChunks.y * worldGrid.chunkDimensionsInCells.y * worldGrid.cellSize;

    std::vector<sf::Vector2f> points;
    std::mt19937 rng(static_cast<unsigned int>(_params.seed));
    std::uniform_real_distribution<float> distX(worldWidth * 0.2f, worldWidth * 0.8f);
    std::uniform_real_distribution<float> distY(worldHeight * 0.2f, worldHeight * 0.8f);

    // Generate a set of random points
    int numPoints = Constants::ISLAND_BASE_SHAPE_POINTS; // More points = more complex base shape
    for (int i = 0; i < numPoints; ++i) {
        points.push_back({distX(rng), distY(rng)});
    }

    // Find the centroid
    sf::Vector2f centroid(0, 0);
    for (const auto& p : points) {
        centroid += p;
    }
    centroid.x /= numPoints;
    centroid.y /= numPoints;

    // Sort points by angle around the centroid to form a simple (non-self-intersecting) polygon
    std::sort(points.begin(), points.end(), [&](const sf::Vector2f& a, const sf::Vector2f& b) {
        return atan2(a.y - centroid.y, a.x - centroid.x) < atan2(b.y - centroid.y, b.x - centroid.x);
    });

    return points;
}

std::vector<sf::Vector2f> WorldGenerationSystem::distortCoastline(const std::vector<sf::Vector2f>& baseShape) {
    std::vector<sf::Vector2f> distortedShape;
    if (baseShape.size() < 2) {
        return baseShape;
    }

    // Noise parameters for distortion. These should be different from the main generation
    // to ensure the distortion feels like a separate, finer detail.
    float distortionFrequency = Constants::COASTLINE_DISTORTION_FREQUENCY; // Higher frequency for more jaggedness
    float distortionStrength = Constants::COASTLINE_DISTORTION_STRENGTH;  // How far vertices can be pushed, in world units

    for (size_t i = 0; i < baseShape.size(); ++i) {
        const sf::Vector2f& p1 = baseShape[i];
        const sf::Vector2f& p2 = baseShape[(i + 1) % baseShape.size()]; // Wrap around to close the loop

        distortedShape.push_back(p1);

        sf::Vector2f segment = p2 - p1;
        float segmentLength = std::sqrt(segment.x * segment.x + segment.y * segment.y);
        sf::Vector2f direction = segment / segmentLength;
        sf::Vector2f normal = {-direction.y, direction.x}; // Perpendicular vector

        // Subdivide the segment to add detail
        int subdivisions = static_cast<int>(segmentLength / Constants::COASTLINE_SUBDIVISION_LENGTH); // Create a vertex roughly every 20 pixels
        for (int j = 1; j < subdivisions; ++j) {
            float t = static_cast<float>(j) / subdivisions;
            sf::Vector2f pointOnEdge = p1 + direction * (segmentLength * t);

            // Get noise value for this point
            float noiseValue = _noiseGenerator.GetNoise(pointOnEdge.x * distortionFrequency, pointOnEdge.y * distortionFrequency);

            // Displace the point along the normal
            sf::Vector2f displacedPoint = pointOnEdge + normal * noiseValue * distortionStrength;
            distortedShape.push_back(displacedPoint);
        }
    }

    return distortedShape;
}

void WorldGenerationSystem::onRegenerateWorldRequest(const RegenerateWorldRequestEvent& event) {
    LOG_INFO("WorldGenerationSystem", "Regenerate World request received.");
    auto view = _registry.view<WorldGridComponent>();
    if (!view.empty()) {
        auto& worldGrid = view.get<WorldGridComponent>(view.front());
        generateWorld(worldGrid.worldDimensionsInChunks.x, worldGrid.worldDimensionsInChunks.y);
    }
}

void WorldGenerationSystem::onWorldGenParamsChange(const WorldGenParamsChangeEvent& event) {
    LOG_INFO("WorldGenerationSystem", "World generation parameters updated.");
    setParams(event.params);

    auto view = _registry.view<WorldGridComponent>();
    if (!view.empty()) {
        auto& worldGrid = view.get<WorldGridComponent>(view.front());
        worldGrid.worldDimensionsInChunks = {event.worldChunksX, event.worldChunksY};
        worldGrid.chunkDimensionsInCells = {event.chunkSizeX, event.chunkSizeY};
        worldGrid.cellSize = event.cellSize;
    }
}