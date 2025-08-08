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

void TerrainRenderSystem::render(entt::registry& registry, sf::RenderTarget& target, const sf::View& view) {
    const WorldGridComponent& worldGrid = getWorldGridSettings(registry);
    auto chunkView = registry.view<ChunkComponent>();

    // Calculate the view's bounding box using the SFML 3 API
    sf::FloatRect viewBounds({view.getCenter() - view.getSize() / 2.f, view.getSize()});

    // Optional: Inflate the view bounds slightly to prevent pop-in at the edges
    viewBounds.position.x -= worldGrid.cellSize;
    viewBounds.position.y -= worldGrid.cellSize;
    viewBounds.size.x += worldGrid.cellSize * 2;
    viewBounds.size.y += worldGrid.cellSize * 2;

    int renderedChunkCount = 0;
    for (auto entity : chunkView) {
        auto& chunk = chunkView.get<ChunkComponent>(entity);

        // Calculate the chunk's bounding box
        float chunkWidthPixels = worldGrid.chunkDimensionsInCells.x * worldGrid.cellSize;
        float chunkHeightPixels = worldGrid.chunkDimensionsInCells.y * worldGrid.cellSize;
        sf::FloatRect chunkBounds(
            {chunk.chunkGridPosition.x * chunkWidthPixels, chunk.chunkGridPosition.y * chunkHeightPixels},
            {chunkWidthPixels, chunkHeightPixels}
        );

        // The culling check remains the same, as intersects() is still the correct method
        if (!viewBounds.findIntersection(chunkBounds)) {
            continue;
        }

        // If the chunk is visible, proceed with rendering
        if (chunk.isMeshDirty) {
            buildChunkMesh(chunk, worldGrid);
        }
        target.draw(chunk.vertexArray);

        if (_visualizeNoiseStateChanged) {
            auto chunkView = registry.view<ChunkComponent>();
            for (auto entity : chunkView) {
                chunkView.get<ChunkComponent>(entity).isMeshDirty = true;
            }
            _visualizeNoiseStateChanged = false;
        }
        
        // --- START VISUALIZATION LOGIC ---
        if (_visualizeChunkBorders) {
            float left = chunkBounds.position.x;
            float top = chunkBounds.position.y;
            float right = left + chunkBounds.size.x;
            float bottom = top + chunkBounds.size.y;

            sf::VertexArray border(sf::PrimitiveType::LineStrip, 5);
            border[0].position = {left, top};
            border[0].color = sf::Color::Red;
            border[1].position = {right, top};
            border[1].color = sf::Color::Red;
            border[2].position = {right, bottom};
            border[2].color = sf::Color::Red;
            border[3].position = {left, bottom};
            border[3].color = sf::Color::Red;
            border[4].position = {left, top}; // Close the loop
            border[4].color = sf::Color::Red;

            target.draw(border);
        }

        if (_visualizeCellBorders) {
            sf::VertexArray gridLines(sf::PrimitiveType::Lines);
            float chunkLeft = chunkBounds.position.x;
            float chunkTop = chunkBounds.position.y;
            float chunkRight = chunkLeft + chunkBounds.size.x;
            float chunkBottom = chunkTop + chunkBounds.size.y;
            sf::Color gridColor(128, 128, 128, 128);

            // Vertical lines
            for (int i = 1; i < worldGrid.chunkDimensionsInCells.x; ++i) {
                float x = chunkLeft + i * worldGrid.cellSize;
                sf::Vertex topVertex, bottomVertex;
                
                topVertex.position = {x, chunkTop};
                topVertex.color = gridColor;
                
                bottomVertex.position = {x, chunkBottom};
                bottomVertex.color = gridColor;
                
                gridLines.append(topVertex);
                gridLines.append(bottomVertex);
            }
            // Horizontal lines
            for (int i = 1; i < worldGrid.chunkDimensionsInCells.y; ++i) {
                float y = chunkTop + i * worldGrid.cellSize;
                sf::Vertex leftVertex, rightVertex;

                leftVertex.position = {chunkLeft, y};
                leftVertex.color = gridColor;

                rightVertex.position = {chunkRight, y};
                rightVertex.color = gridColor;

                gridLines.append(leftVertex);
                gridLines.append(rightVertex);
            }
            target.draw(gridLines);
        }
        // --- END VISUALIZATION LOGIC ---

        renderedChunkCount++;
    }
    // LOG_TRACE("TerrainRenderSystem", "Rendered %d visible chunks.", renderedChunkCount);
}

void TerrainRenderSystem::buildChunkMesh(ChunkComponent& chunk, const WorldGridComponent& worldGrid) {
    if (chunk.vertexArray.getVertexCount() == 0) {
        chunk.vertexArray.resize(worldGrid.chunkDimensionsInCells.x * worldGrid.chunkDimensionsInCells.y * 6);
    }

    auto updateCell = [&](int cellIndex) {
        int y = cellIndex / worldGrid.chunkDimensionsInCells.x;
        int x = cellIndex % worldGrid.chunkDimensionsInCells.x;

        sf::Vertex* tri = &chunk.vertexArray[cellIndex * 6];

        float screenX = (chunk.chunkGridPosition.x * worldGrid.chunkDimensionsInCells.x + x) * worldGrid.cellSize;
        float screenY = (chunk.chunkGridPosition.y * worldGrid.chunkDimensionsInCells.y + y) * worldGrid.cellSize;

        sf::Vector2f topLeft(screenX, screenY);
        sf::Vector2f topRight(screenX + worldGrid.cellSize, screenY);
        sf::Vector2f bottomLeft(screenX, screenY + worldGrid.cellSize);
        sf::Vector2f bottomRight(screenX + worldGrid.cellSize, screenY + worldGrid.cellSize);

        tri[0].position = topLeft;
        tri[1].position = topRight;
        tri[2].position = bottomLeft;
        tri[3].position = topRight;
        tri[4].position = bottomRight;
        tri[5].position = bottomLeft;

        sf::Color color;
        if (_visualizeNoise) {
            float rawNoiseValue = chunk.rawNoiseValues[cellIndex];
            float normalizedNoise = std::max(0.0f, std::min(1.0f, rawNoiseValue));
            uint8_t gray = static_cast<uint8_t>(normalizedNoise * 255);
            color = sf::Color(gray, gray, gray);
        } else {
            switch (chunk.cells[cellIndex]) {
                case TerrainType::WATER: color = sf::Color(173, 216, 230); break;
                case TerrainType::LAND: color = sf::Color(34, 139, 34); break;
                case TerrainType::RIVER: color = sf::Color(100, 149, 237); break;
                default: color = sf::Color::Magenta; break;
            }
        }

        for (int i = 0; i < 6; ++i) {
            tri[i].color = color;
        }
    };

    if (chunk.dirtyCells.empty()) {
        int totalCells = worldGrid.chunkDimensionsInCells.x * worldGrid.chunkDimensionsInCells.y;
        for (int i = 0; i < totalCells; ++i) {
            updateCell(i);
        }
    } else {
        for (int cellIndex : chunk.dirtyCells) {
            updateCell(cellIndex);
        }
    }

    chunk.dirtyCells.clear();
    chunk.isMeshDirty = false;
}