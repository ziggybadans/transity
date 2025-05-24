#include "TerrainRenderSystem.h"
#include <iostream> // For debugging if needed
#include <algorithm>
#include <cstdint>

TerrainRenderSystem::TerrainRenderSystem() : _visualizeNoise(false) {
    // Initialize cellShape once. Its size will be set from WorldGridComponent.
    // Its position and color will be set per cell.
}

const WorldGridComponent& TerrainRenderSystem::getWorldGridSettings(entt::registry& registry) {
    auto view = registry.view<WorldGridComponent>();
    if (view.empty()) {
        throw std::runtime_error("TerrainRenderSystem: WorldGridComponent not found in registry!");
    }
    return view.get<WorldGridComponent>(view.front());
}

void TerrainRenderSystem::render(entt::registry& registry, sf::RenderTarget& target) {
    const WorldGridComponent& worldGrid = getWorldGridSettings(registry);
    
    _cellShape.setSize(sf::Vector2f(worldGrid.cellSize, worldGrid.cellSize));
    _cellShape.setOutlineThickness(0); // Or a small value if you want grid lines

    auto chunkView = registry.view<ChunkComponent>(); // Get all entities with a ChunkComponent

    for (auto entity : chunkView) {
        const ChunkComponent& chunk = chunkView.get<ChunkComponent>(entity);

        for (int y = 0; y < worldGrid.chunkDimensionsInCells.y; ++y) {
            for (int x = 0; x < worldGrid.chunkDimensionsInCells.x; ++x) {
                int cellIndex = y * worldGrid.chunkDimensionsInCells.x + x;
                TerrainType type = chunk.cells[cellIndex];

                // Calculate screen position for the cell
                float screenX = (chunk.chunkGridPosition.x * worldGrid.chunkDimensionsInCells.x + x) * worldGrid.cellSize;
                float screenY = (chunk.chunkGridPosition.y * worldGrid.chunkDimensionsInCells.y + y) * worldGrid.cellSize;
                
                _cellShape.setPosition({screenX, screenY});

                if (_visualizeNoise) {
                    float rawNoiseValue = chunk.noiseValues[cellIndex];
                    float normalizedNoise = (rawNoiseValue + 1.0f) / 2.0f; // Normalize to [0, 1]
                    normalizedNoise = std::max(0.0f, std::min(1.0f, normalizedNoise)); // Clamp to [0, 1]
                    std::uint8_t gray = static_cast<std::uint8_t>(normalizedNoise * 255);
                    _cellShape.setFillColor(sf::Color(gray, gray, gray)); // Grayscale based on noise value
                } else {
                    switch (type) {
                        case TerrainType::WATER:
                            _cellShape.setFillColor(sf::Color::Blue);
                            break;
                        case TerrainType::LAND:
                            _cellShape.setFillColor(sf::Color(34, 139, 34)); // Forest Green
                            break;
                        case TerrainType::RIVER: // We'll add this later, but good to have a case
                            _cellShape.setFillColor(sf::Color(100, 149, 237)); // Cornflower Blue
                            break;
                        default:
                            _cellShape.setFillColor(sf::Color::Magenta); // Error/Unknown
                            break;
                    }
                }
                target.draw(_cellShape);
            }
        }
    }
}
