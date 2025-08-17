World Generation Design Document — Constraint‑Aware, Modular Pipeline

Executive summary
- Purpose: Introduce a flexible, layered world generation system that independently generates and tags major features (cliffs, mountain peaks, rivers, lakes, forests, oceans) while maintaining a hidden but general elevation‑based biome classification (hills, mountains, flats, valleys, coastlines) for gameplay costs and flavor.
- Approach: Implement a staged, constraint‑aware pipeline that reads and writes shared fields per cell and sets feature tags. Keep features independent and resolve conflicts via soft rules. The system integrates into the existing chunked ECS and rendering.
- Deliverable: A step‑by‑step plan describing exactly which files to add or edit, in what order, and how to validate results, aligned to the current codebase.

References to current codebase
- Generation
  - World settings, noise layers and thresholds are defined under [WorldData.h](src/world/WorldData.h).
  - Noise configuration and continent shape seeding in [WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp:37) and [WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp:60).
  - Per‑chunk generation entrypoint starts at [WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp:107), outputting TerrainType per cell plus two noise arrays.
  - System definition in [WorldGenerationSystem.h](src/systems/world/WorldGenerationSystem.h).
- Chunk lifecycle
  - Chunk async load, finalize, and component assembly in [ChunkManagerSystem.cpp](src/systems/world/ChunkManagerSystem.cpp:179).
  - Components that exist today are defined in [WorldComponents.h](src/components/WorldComponents.h).
- Rendering
  - Terrain mesh build by cell‑type rectangle merging and LOD in [TerrainRenderSystem.cpp](src/systems/rendering/TerrainRenderSystem.cpp:116).
  - Terrain drawing with chunk bounds culling in [TerrainRenderSystem.cpp](src/systems/rendering/TerrainRenderSystem.cpp:42).
  - Mesh update integration in [TerrainMeshSystem.cpp](src/systems/rendering/TerrainMeshSystem.cpp).
  - Frame composition in [Renderer.cpp](src/render/Renderer.cpp:32).

Observed gaps vs target
- No persistent elevation or derived fields per cell. Only TerrainType LAND or WATER and raw noise are stored.
- No hydrology model [no fill, flow, accumulation, rivers, lakes].
- No cliffs, peaks, moisture, temperature, forests, or biome classification.
- Rendering assumes a single base TerrainType for color. There is no feature overlay pass.
- Parameters exist but are focused on noise layers and coastline distortion, not a full style.

Design goals
- Keep features independent and modular. Features read shared fields and set tags, not overwrite each other.
- Provide a single WorldStyle parameter struct governing style and reproducibility [seeds, scales, thresholds].
- Generate in tiles [chunks], cache borders as needed, and keep all generation deterministic per seed.
- Support debug overlays for slope, flow accumulation, moisture, temperature, and feature tags.

Data model changes [per cell, per chunk]
Store the following fields SoA [structure of arrays] in a new chunk component. Use float for simplicity first; later optimize to uint16 quantization for some fields.
- Scalars per cell:
  - elev [meters relative to sea level 0 after normalization]
  - slope [radians or degrees; choose consistent unit]
  - aspect [angle 0..2pi or None if slope near zero]
  - moisture [0..1]
  - temperature [C or normalized 0..1]
  - flowAccum [flow accumulation count or normalized log scale]
  - baseLevel [ocean base level around 0, or local base for erosion guard]
  - relief [local relief within N cells neighborhood]
  - distCoast [in world units or cells]
  - soilDepth [0..1 or meters for hardness proxy]
- Tags per cell:
  - tags bitset uint32 with bits WATER, RIVER, LAKE, COAST, CLIFF, PEAK, FOREST [matching PLAN_WORLD_GEN.md].
- Optional:
  - biomeId small integer [hidden classification used by gameplay, not rendering base colors].

ECS component additions
- New component: ChunkFieldsComponent [one instance per chunk]
  - Arrays [vector sized to chunkCells] for all fields listed above.
  - tags as a vector of uint32 per cell.
  - resize function to size all arrays to chunkDimensionsInCells.x * chunkDimensionsInCells.y.
- Retain ChunkTerrainComponent to hold coarse TerrainType for base land vs water for now, to keep rendering simple and fast.
- Retain ChunkNoiseComponent initially as a debug aid, can be removed later.

Where to put the new files
- Add a new folder: src/worldgen
  - Files to add:
    - src/worldgen/WorldStyle.h [defines the parameter struct and sensible defaults]
    - src/worldgen/GeneratorContext.h [lightweight struct referencing registry, grid settings, style, RNG seeding helpers]
    - src/worldgen/GenNode.h [base interface for pipeline nodes]
    - src/worldgen/ElevationNode.h and src/worldgen/ElevationNode.cpp
    - src/worldgen/HydroNode.h and src/worldgen/HydroNode.cpp
    - src/worldgen/CoastCliffNode.h and src/worldgen/CoastCliffNode.cpp
    - src/worldgen/PeaksNode.h and src/worldgen/PeaksNode.cpp
    - src/worldgen/ClimateNode.h and src/worldgen/ClimateNode.cpp
    - src/worldgen/ForestNode.h and src/worldgen/ForestNode.cpp
    - src/worldgen/ResolveNode.h and src/worldgen/ResolveNode.cpp
    - src/worldgen/GeneratorGraph.h and src/worldgen/GeneratorGraph.cpp [composes nodes in order]
  - Optional helpers:
    - src/worldgen/NoiseUtils.h and src/worldgen/NoiseUtils.cpp [domain warp, ridge noise]
    - src/worldgen/HydroUtils.h and src/worldgen/HydroUtils.cpp [priority flood, D8 or D∞ flow, accumulation]
    - src/worldgen/Sampling.h and src/worldgen/Sampling.cpp [Poisson disk, blue noise helper]
- Add a new component header:
  - src/components/WorldFieldsComponent.h [defines ChunkFieldsComponent]. Include this where needed.

WorldStyle parameter struct [data only, not code]
- Purpose: Centralize parameters controlling world style and reproducibility. Accessible globally, with per‑node seeds derived from WorldStyle seed + node id.
- Suggested fields [initial set]:
  - seed [uint32]
  - seaLevel [float in elevation units after normalization, default 0.0]
  - continentality [0..1 controlling low‑freq land coverage]
  - plateCount [integer for Voronoi tectonics mask, default small 6..12]
  - orogenStrength [0..1 scaling ridge intensity]
  - riverThreshold [flowAccum threshold for river carving]
  - erosionIterations [0..N small integer for river carve loop]
  - lakeMinArea [cells threshold for classifying closed basins as lakes]
  - cliffSlopeThreshold [degrees or radians]
  - cliffReliefThreshold [units across N cell radius]
  - coastalBand [cells distance from sea to mark COAST]
  - windDir [2D unit vector]
  - lapseRate [temperature drop per elevation unit]
  - rainShadowStrength [0..1]
  - forestDensity [0..1]
  - treeLineAlt [elevation cutoff for forests]
  - warpScaleLow, warpScaleHigh [domain warping magnitudes]
  - ridgeGain and octaveCount for ridged multifractal shaping
- Storage:
  - Persist inside WorldStateComponent in addition to or replacing the existing WorldGenParams, so events can swap active and generating styles in [WorldComponents.h](src/components/WorldComponents.h).

Pipeline stages [what each node reads and writes]
ElevationNode
- Inputs: WorldStyle, grid coordinates, FastNoiseLite seeded from WorldStyle.
- Method:
  - Produce base elev using ridged multifractal, domain warp [warpScaleLow and warpScaleHigh], and low‑frequency tectonic Voronoi bias for ranges and island arcs.
  - Normalize such that seaLevel ends up around 0.0 [shift and scale].
  - Initialize baseLevel ~ 0.0 for ocean; write elev per cell as float.
- Outputs:
  - elev[]
  - baseLevel[] initial
  - Optional: initial WATER tag by elev < seaLevel for downstream convenience.

HydroNode
- Inputs: elev[], seaLevel, WorldStyle hydrology params.
- Method:
  - Priority‑flood fill to remove pits and identify basins; label lakes where closed basins exceed lakeMinArea.
  - Compute flow direction [D8 or D∞] per cell.
  - Compute flowAccum per cell [consider log normalization].
  - Carve rivers where flowAccum exceeds riverThreshold, lowering elev along path and slightly increasing relief near channel; repeat erosionIterations times.
  - Recompute slope after carving.
- Outputs:
  - flowAccum[], updated elev[]
  - lake tag on cells in basins; river tag on channel cells.

CoastCliffNode
- Inputs: elev[], seaLevel, slope[], relief[].
- Method:
  - COAST tag for cells within coastalBand of a sea cell [flood fill distance; can be approximated by distance transform on WATER].
  - CLIFF tag where slope exceeds cliffSlopeThreshold and neighborhood relief exceeds cliffReliefThreshold across N cells.
  - Optional: slightly warp elev near coast to produce fjords or stacks, gated by style toggles.
- Outputs:
  - tags updated for COAST and CLIFF; relief[] computed if not already.

PeaksNode
- Inputs: elev[], optional tectonic mask, style parameters.
- Method:
  - Find local maxima of a high‑frequency peak noise masked by high elevation [elev above peakMinAlt].
  - Use Poisson disk sampling to enforce spacing.
  - Tag PEAK cells and boost relief locally via radial falloff around peaks.
- Outputs:
  - tags updated for PEAK; relief[] amplified near peaks.

ClimateNode
- Inputs: latitude proxy [derive from world coordinates], elev[], slope[], aspect[], windDir, lapseRate.
- Method:
  - temperature from latitude and lapse rate vs elevation.
  - moisture as a function of distCoast and prevailing wind; add orographic rain [increase on windward slopes where wind dot normal is positive].
  - Smooth moisture and temperature lightly for coherence.
- Outputs:
  - temperature[], moisture[], distCoast[].

ForestNode
- Inputs: moisture[], temperature[], soilDepth[] [initialize to constant if not modeled yet], treeLineAlt.
- Method:
  - Compute forest probability p = f moisture, temperature, soilDepth with veto above treeLineAlt.
  - Use blue‑noise sampling to set FOREST tag on selected cells according to p to avoid carpet coverage.
- Outputs:
  - tags updated for FOREST.

ResolveNode
- Inputs: all tags and fields.
- Method:
  - Enforce soft constraints [rivers descend, lakes have outflow unless endorheic; peaks at high elevation; forests below tree line].
  - If conflicts remain [rare], use priority or soft costs to break ties per cell and finalize tags, without overwriting existing valid features.

Integration with current systems
WorldGenerationSystem
- Keep as the orchestrator called by chunk jobs. Replace the current direct noise‑to‑TerrainType approach at [WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp:107) with:
  - Construct a GeneratorContext holding references to WorldStyle, grid info, chunk cell arrays, and seeded RNG utilities.
  - Instantiate and run node sequence [Elevation, Hydro, CoastCliff, Peaks, Climate, Forest, Resolve].
  - Fill:
    - ChunkFieldsComponent with elev, slope, aspect, moisture, temperature, flowAccum, baseLevel, relief, distCoast, soilDepth and tags.
    - ChunkTerrainComponent.basemap as LAND or WATER using elev vs seaLevel. Optional: set TerrainType RIVER or LAKE where tags indicate [or keep rivers and lakes as overlays only, see rendering].
  - Keep ChunkNoiseComponent for now as a debug artifact for visual comparisons.

ChunkManagerSystem
- In [ChunkManagerSystem.cpp](src/systems/world/ChunkManagerSystem.cpp:179):
  - After creating entity and emplacing ChunkPositionComponent:
    - Emplace ChunkFieldsComponent populated from new generation output.
    - Keep emplace of ChunkTerrainComponent and ChunkNoiseComponent as today to avoid breaking rendering.
  - No changes to async orchestration are required beyond carrying the larger GeneratedChunkData payload.

Rendering and overlays
- Base layer: Keep existing rectangle‑merge mesh for TerrainType LAND and WATER in [TerrainRenderSystem.cpp](src/systems/rendering/TerrainRenderSystem.cpp:116).
- Overlays: Add separate overlay meshes per LOD for features whose tags are independent of TerrainType:
  - Rivers and lakes [draw as semi‑transparent blue with high z order above land]
  - Forests [draw as semi‑transparent green stipple or darker green tint quads]
  - Coasts [optional thin band highlight near water cells]
  - Cliffs [optional dark edge or diagonal hatch overlay where CLIFF tag is set]
- Implementation strategy:
  - Extend ChunkMeshComponent or add ChunkOverlayMeshComponent holding a small fixed set of VertexArray lists per LOD, keyed by feature.
  - Build overlay meshes during buildAllChunkMeshes pass with the same rectangle merging, but disallow merges across differing overlay presence. For each feature overlay, run a separate rectangle‑merge sweep.
  - Add TerrainRenderSystem toggles to enable or disable overlays.
- Debug overlays:
  - Implement view modes where base layer color is remapped to show slope, flowAccum, moisture, temperature via color ramps. Add toggles similar to existing chunk and cell border toggles in [TerrainRenderSystem.h](src/systems/rendering/TerrainRenderSystem.h).
  - Expose toggles via a small debug UI [if you have a UI layer] or temporary keyboard binds.

Seeds and reproducibility
- WorldStyle.seed is the only top‑level seed. Each node derives its seed as hash seed, nodeId, 32 bit mixing to avoid cross‑node correlations.
- GeneratorContext provides a method getNodeRng nodeId that returns a stateless RNG seeded deterministically.

Performance considerations
- Generation remains chunk‑local. For hydrology across chunk borders:
  - Cache border elevations, flow directions, and basin labels for neighbor consistency. When generating a chunk, request the generation of immediate neighbors borders if not already computed, or delay finalize until border data is available.
  - Use a 1 cell halo for slope, relief, and distance transforms to avoid artifacts at chunk seams.
- Keep arrays SoA for CPU cache and potential SIMD later.
- Quantize elev to uint16 with a scale if memory becomes a concern.
- Continue to use async chunk generation via the existing thread pool pathway in [ChunkManagerSystem.cpp](src/systems/world/ChunkManagerSystem.cpp).

Step by step implementation plan [non‑coding execution guide]
Preparation
1. Read and familiarize with:
   - [WorldData.h](src/world/WorldData.h)
   - [WorldGenerationSystem.h](src/systems/world/WorldGenerationSystem.h)
   - [WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp)
   - [ChunkManagerSystem.h](src/systems/world/ChunkManagerSystem.h)
   - [ChunkManagerSystem.cpp](src/systems/world/ChunkManagerSystem.cpp)
   - [TerrainRenderSystem.h](src/systems/rendering/TerrainRenderSystem.h)
   - [TerrainRenderSystem.cpp](src/systems/rendering/TerrainRenderSystem.cpp)
2. Create a new folder: src/worldgen

Data and components
3. Add a new header file: src/components/WorldFieldsComponent.h
   - Define ChunkFieldsComponent with arrays [elev, slope, aspect, moisture, temperature, flowAccum, baseLevel, relief, distCoast, soilDepth] and tags [uint32].
   - Provide a resize method that resizes all arrays to chunkCellCount.
4. Include WorldFieldsComponent.h wherever chunk components are used [ChunkManagerSystem.cpp, TerrainRenderSystem.cpp].
5. Add a new header file: src/worldgen/WorldStyle.h
   - Define WorldStyle with the parameter list described earlier plus sensible defaults.
   - Ensure this can be copied and stored inside WorldStateComponent.
6. In [WorldComponents.h](src/components/WorldComponents.h), extend WorldStateComponent to store WorldStyle if not reusing WorldGenParams. If you retain WorldGenParams for noise presets, add WorldStyle alongside it.

Generator infrastructure
7. Add src/worldgen/GeneratorContext.h
   - Minimal structure holding references to WorldStyle, grid settings, and accessors for seeding RNG per node.
8. Add src/worldgen/GenNode.h
   - Describe a base node interface with a run operation that reads and writes chunk fields inside ChunkFieldsComponent via the context.
9. Add node files [headers and sources] for Elevation, Hydro, CoastCliff, Peaks, Climate, Forest, Resolve as listed under Where to put the new files. Each node is small and only manipulates arrays described above.
10. Add src/worldgen/GeneratorGraph.h and .cpp
   - Compose nodes in order and expose a single runChunk function that executes the pipeline nodes for a given chunk and context.

World generation integration
11. In [WorldGenerationSystem.h](src/systems/world/WorldGenerationSystem.h), add includes for GeneratorGraph and WorldFieldsComponent.
12. In [WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp:107), replace the current per‑cell noise and threshold routine with:
    - Allocate a ChunkFieldsComponent for the chunk [local data during generation].
    - ElevationNode run [compute elev normalization to sea level].
    - HydroNode run [priority flood, flow, accumulation, carve, re‑slope].
    - CoastCliffNode run [coastal band and cliff detection].
    - PeaksNode run [peak candidates, spacing, tag, relief amplification].
    - ClimateNode run [temperature, moisture, distCoast].
    - ForestNode run [probability and blue‑noise selection].
    - ResolveNode run [soft constraints and tag tie‑breaks].
    - Decide TerrainType per cell for base layer [LAND if elev >= seaLevel else WATER]. Optionally set RIVER or LAKE TerrainType where tags indicate, or keep them as overlays only for now.
    - Package GeneratedChunkData plus the new fields and tags so that ChunkManagerSystem can emplace ChunkFieldsComponent.
13. Keep configureNoise and generateContinentShape where they are [they can be repurposed inside ElevationNode or left as a pre‑mask]. Link style parameters to these functions.

Chunk manager integration
14. In [ChunkManagerSystem.cpp](src/systems/world/ChunkManagerSystem.cpp:179), after creating the chunk entity:
    - Emplace ChunkFieldsComponent with the arrays and tags produced by the generator.
    - Keep existing emplace calls for ChunkTerrainComponent and ChunkNoiseComponent unchanged.
15. No other chunk lifecycle changes are required.

Rendering and overlays
16. Extend ChunkMeshComponent or add ChunkOverlayMeshComponent to hold overlay vertex arrays per LOD for FEATURES [RIVER, LAKE, FOREST, CLIFF, COAST].
17. In [TerrainRenderSystem.h](src/systems/rendering/TerrainRenderSystem.h), add toggles to enable overlay building and debug modes [slope, flow accumulation, moisture, temperature].
18. In [TerrainRenderSystem.cpp](src/systems/rendering/TerrainRenderSystem.cpp:116):
    - Keep the base mesh building for TerrainType rectangles.
    - Add a second build pass per enabled overlay tag:
      - Run the same rectangle‑merge algorithm but consider cells as the same only if their overlay presence matches.
      - Choose distinct colors and alpha for each overlay [rivers deep blue, lakes bright blue, forests semi‑transparent dark green, coasts sand‑yellow, cliffs dark gray].
    - If debug mode is set, remap base color by the chosen scalar field [e g slope or flowAccum] using simple ramps in code for fast visual inspection.
19. In [Renderer.cpp](src/render/Renderer.cpp:32), the TerrainRenderSystem render call remains the same. Overlays draw within the same pass as additional vertex arrays.

Presets and parameters
20. Add a default WorldStyle preset [Earthlike] saved within WorldStyle.h for quick boot.
21. Expose the sea level and a few key toggles [erosionIterations, riverThreshold, forestDensity] through your UI if available, or via a simple config file read on boot in Application initialization.

Validation and debugging
22. Add PerfTimer scopes around all new generation stages to keep track of costs [see [PerfTimer.h](src/core/PerfTimer.h)].
23. Add a debug mode that draws each field:
    - Slope map
    - Flow accumulation [log scaled]
    - Moisture
    - Temperature
    - Relief
    - Tag masks
24. Sanity checks:
    - Rivers only run downhill and terminate at lakes or sea.
    - Lakes either have a valid spillway or are flagged endorheic [optional].
    - Peaks appear at high elevation and respect spacing.
    - Forests do not appear above tree line and correlate with moisture.

Performance milestones
- Milestone 1 [base]: Elevation and sea level only, with LAND and WATER base map, overlays disabled.
- Milestone 2 [hydro]: Add hydrology and draw rivers and lakes as overlays. Ensure seam‑free behavior between neighbor chunks at the camera center by generating a halo on demand.
- Milestone 3 [cliffs and peaks]: Tag and overlay cliffs and peaks. Validate relief exaggeration yields nice silhouettes at current zooms.
- Milestone 4 [climate and forests]: Temperature, moisture, distCoast; forest placement via blue noise; draw forest overlay.
- Milestone 5 [resolve and polish]: Add soft constraint resolver, add presets, and finalize debug overlays.

Risks and mitigations
- Chunk seam artifacts in hydrology: Generate a 1 cell halo and cache neighbor borders; use deterministic seeding for consistent borders.
- Overdraw from overlay meshes: Use rectangle merges for overlays like base; let users toggle overlays off for performance.
- Memory growth: Keep fields SoA; consider quantizing elev and flowAccum to uint16 when profiling indicates pressure.
- Complexity creep: Keep nodes small and pure [stateless run given inputs and outputs].

Mermaid diagrams
Pipeline DAG
flowchart TD
  A[WorldStyle params and seed] --> B[ElevationNode outputs elev and baseLevel]
  B --> C[HydroNode outputs flowAccum and updates elev and lake and river tags]
  C --> D[CoastCliffNode outputs coast and cliff tags and relief]
  D --> E[PeaksNode outputs peak tags and boosts relief]
  E --> F[ClimateNode outputs temperature and moisture and distCoast]
  F --> G[ForestNode outputs forest tags]
  G --> H[ResolveNode returns final tags]
  H --> I[ChunkFieldsComponent stored on chunk entity]

ECS and rendering data flow
flowchart LR
  J[WorldGenerationSystem generate chunk] --> K[ChunkManagerSystem emplaces components]
  K --> L[ChunkFieldsComponent fields and tags]
  K --> M[ChunkTerrainComponent base types]
  L --> N[TerrainRenderSystem overlay mesh build]
  M --> O[TerrainRenderSystem base mesh build]
  N --> P[Renderer draws overlays]
  O --> P[Renderer draws base]

File by file editing checklist [exact file paths]
Create
- src/components/WorldFieldsComponent.h
- src/worldgen/WorldStyle.h
- src/worldgen/GeneratorContext.h
- src/worldgen/GenNode.h
- src/worldgen/ElevationNode.h and src/worldgen/ElevationNode.cpp
- src/worldgen/HydroNode.h and src/worldgen/HydroNode.cpp
- src/worldgen/CoastCliffNode.h and src/worldgen/CoastCliffNode.cpp
- src/worldgen/PeaksNode.h and src/worldgen/PeaksNode.cpp
- src/worldgen/ClimateNode.h and src/worldgen/ClimateNode.cpp
- src/worldgen/ForestNode.h and src/worldgen/ForestNode.cpp
- src/worldgen/ResolveNode.h and src/worldgen/ResolveNode.cpp
- src/worldgen/GeneratorGraph.h and src/worldgen/GeneratorGraph.cpp

Edit
- [src/components/WorldComponents.h](src/components/WorldComponents.h) to extend WorldStateComponent with WorldStyle or migrate to it.
- [src/systems/world/WorldGenerationSystem.h](src/systems/world/WorldGenerationSystem.h) to reference the generator graph and world fields.
- [src/systems/world/WorldGenerationSystem.cpp](src/systems/world/WorldGenerationSystem.cpp:107) to replace direct noise thresholding with graph execution and fields population.
- [src/systems/world/ChunkManagerSystem.cpp](src/systems/world/ChunkManagerSystem.cpp:179) to emplace the new ChunkFieldsComponent during processCompletedChunks.
- [src/systems/rendering/TerrainRenderSystem.h](src/systems/rendering/TerrainRenderSystem.h) to add overlay toggles and optional debug modes.
- [src/systems/rendering/TerrainRenderSystem.cpp](src/systems/rendering/TerrainRenderSystem.cpp:116) to build overlay meshes and debug views.
- Optional: [src/render/Renderer.cpp](src/render/Renderer.cpp:32) to wire any new overlay visualization toggles if exposed via events or UI.

Acceptance criteria
- Visual: Oceans and continents form naturally warped coasts. Rivers start at higher elevations, join tributaries, and reach lakes or the sea. Cliffs appear on steep coastal or mountainous edges. Peaks dot high ranges with spacing. Forests cluster in moist low and mid elevations, absent above tree line.
- Data: For any chunk entity, ChunkFieldsComponent arrays match chunk cell count; tags combine properly; debug overlays for slope and flowAccum reflect expected distributions.
- Performance: Generation per chunk completes within a few milliseconds on a modern CPU when executed asynchronously, with overlays toggleable to avoid overdraw.

End of document