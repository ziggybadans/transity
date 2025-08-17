Short version: build a layered, constraint-aware pipeline. Keep features as independent modules that read shared fields (elevation, slope, moisture, temperature, lithology) and write tags or masks. Let soft rules resolve conflicts. Rivers and coasts come from elevation and flow; cliffs from slope and relief; forests from moisture and temperature. Seeded and parameterized for style.

# Targets the game needs

Per cell store:

* elevation, slope, aspect
* moisture, temperature
* flow\_accum, base\_level (ocean), relief, distance\_to\_coast
* soil\_depth or hardness
* tags bitset: WATER, RIVER, LAKE, COAST, CLIFF, PEAK, FOREST, etc.

# Pipeline that stays flexible

1. Base elevation

* Start with ridged multifractal + domain warping for natural continents.
* Add a low-frequency “tectonic plate” Voronoi to bias mountain belts and island arcs.
* Normalize to sea level, expose a sea\_level parameter.

2. Hydrology

* Priority-flood to fill depressions and define lakes.
* Flow direction (D8 or D∞), then flow accumulation.
* Carve rivers along high accumulation using simple hydraulic erosion to cut valleys.
* Recompute slope after carving.

3. Coasts and cliffs

* COAST = cells within coastal\_band of sea.
* CLIFF = slope > cliff\_slope\_threshold and relief within N cells > cliff\_relief\_threshold.
* Optionally enforce sea stacks and fjords by warping elevation near coast.

4. Peaks and ranges

* Candidate peaks = local maxima of a high-frequency “peak” noise masked by plate boundaries and elevation > peak\_min\_alt.
* Enforce min spacing with Poisson disk on the candidate set.
* Tag PEAK and amplify nearby relief with a radial falloff.

5. Moisture and temperature

* Temperature from latitude proxy + altitude lapse rate.
* Moisture from distance\_to\_coast and prevailing wind. Add orographic rain: moisture += max(0, wind\_dot\_aspect) \* slope \* k.
* Smooth both with small kernels for coherence.

6. Biomes and vegetation

* Forest probability = f(moisture, temperature, soil\_depth) with hard vetoes: elevation > tree\_line ⇒ no forest.
* Place FOREST via blue-noise sampling constrained by probability to avoid carpet coverage.
* Assign biome from a Whittaker-style lookup using temperature vs moisture, but do not overwrite feature tags.

7. Final resolve

* Features are independent but constrained by fields:

  * Rivers must descend and cannot cross lakes without an outflow.
  * Peaks require high elevation.
  * Forests require moisture and below tree\_line.
* Use a priority system to break ties, or a soft-cost solver where each feature proposes cells with weights and you pick argmax.

# Why this works for gameplay

* Rivers, lakes, cliffs, and ranges create routing constraints and choke points.
* Valleys, passes, and coastal shelves create obvious corridors for lines and roads.
* Forests give visibility and clearance costs without dictating elevation.
* Biomes give flavor without blocking feature placement.

# Parameters that shape “world style”

Group into a single struct so presets are easy:

* sea\_level, continentality, plate\_count, orogen\_strength
* river\_threshold, erosion\_iterations, lake\_min\_area
* cliff\_slope\_threshold, cliff\_relief\_threshold, coastal\_band
* wind\_dir, lapse\_rate, rain\_shadow\_strength
* forest\_density, tree\_line\_alt
* warp\_scale\_low, warp\_scale\_high, ridge\_gain, octave\_count

# Minimal data model

```cpp
enum Tag : uint32_t { WATER=1<<0, RIVER=1<<1, LAKE=1<<2, COAST=1<<3,
                      CLIFF=1<<4, PEAK=1<<5, FOREST=1<<6 };

struct Cell {
  float elev, slope, aspect;
  float moisture, temp;
  float flow_accum, base_level, relief, dist_coast;
  float soil_depth;
  uint32_t tags;
};

struct World {
  Grid<Cell> g;
  RNG rng;
  WorldStyle style;
};
```

# Generator graph

Define modules with pure inputs and explicit products. Compose as a DAG so you can swap pieces.

```cpp
struct Context { World& w; };

struct GenNode { virtual void run(Context&) = 0; };

struct ElevationNode : GenNode { void run(Context& c) override; };
struct HydroNode      : GenNode { void run(Context& c) override; };
struct CoastCliffNode : GenNode { void run(Context& c) override; };
struct PeaksNode      : GenNode { void run(Context& c) override; };
struct ClimateNode    : GenNode { void run(Context& c) override; };
struct ForestNode     : GenNode { void run(Context& c) override; };
struct ResolveNode    : GenNode { void run(Context& c) override; };

std::vector<std::unique_ptr<GenNode>> graph = {
  make<ElevationNode>(), make<HydroNode>(),
  make<CoastCliffNode>(), make<PeaksNode>(),
  make<ClimateNode>(), make<ForestNode>(),
  make<ResolveNode>()
};
```

# Key algorithms in brief

* Domain warping: sample base noise at p + warp(noise\_low(p))\*warp\_scale. Produces natural coasts.
* Priority-flood: fills pits, then extracts lake basins and spill points in O(N).
* Flow accumulation: parallel scanline or tiled D∞ for smoother rivers.
* Hydraulic erosion lite: along river cells lower elev by k\*log(flow\_accum) and increase relief nearby.
* Poisson disk for peaks and forests: O(N) dart throwing on probability mask to avoid clumps.
* Blue-noise forests: sample against probability then relax with a few Lloyd steps for spacing.

# Performance notes

* Generate in tiles. Cache borders for flow and lakes.
* Keep fields in SoA layout for SIMD.
* Use uint16 heightmaps where possible, float for intermediates.
* Seed every node from a global seed + node id for reproducibility.

# Debugging overlays

Add toggles to render: slope, flow\_accum, relief, moisture, temperature, feature tags.
Export masks to quickly sanity-check constraints.

# Practical guardrails

* Never let erosion drop below base\_level near coasts to avoid submerged rivers.
* Ensure each lake has a spillway unless marked endorheic.
* Enforce a minimum pass density across ranges to avoid dead continents.