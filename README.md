# Transity - Transport Management Simulator
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Overview
Transity is a transport management simulator with elements of a city builder. Unlike games like Cities Skylines and Transport Fever, it is not meant to have realistic graphics, but instead draw players in with a simple, colourful art style combined with a complex, accurate simulation. Transity is inspired by games like Mini Metro and NIMBY Rails but with a desire for the complexity and replayability of Cities Skylines and Transport Fever. While the city's buildings can't be plopped directly like in regular city builders, areas can be zoned and suited to different purposes.

The game's core progression follows this flow:
- Evaluate the world around and change it if desired (eg. terraforming, placing cities, etc.)
- Zone areas of the world in order to give cims a reason to move about
- Create infrastructure (eg. railways, roads, airways, waterways, etc.) and place all necessary structures to support that infrastructure
- Plan routes on that infrastructure using certain modes of transport (eg. bus, tram, train, plane, ferry, highway, etc.)
- Assign vehicles and open projects to traffic to simulate those routes
- Respond to issues like passenger flow, economy, satisfaction, weather, events, etc.

One of the core parts of Transity is endless customisation. You can choose to play in a procedurally generated random world, or choose a place in the world you know, pulled from map data. You can play in a campaign mode where you start one or multiple transport companies and work with the government and public to improve infrastructure, or in a free play mode where you can just build anything you want. You can also turn off any simulation features you don't like (eg. politics, disasters, manual placement of required infrastructure, etc.)

If further customisation is needed, there is a modding API that allows mods to be created covering any part of the game, most notably adding new assets (eg. trains).

## Key Features

- **Comprehensive Transport Simulation**: Road, rail, water, and air networks with advanced pathfinding
- **City Growth Mechanics**: Zoning, population simulation, and economic models
- **Performance Optimized**: Spatial partitioning, multithreading, and data-oriented design
- **Modding Support**: Extensible plugin system for customization
- **SFML Rendering**: Clean visual style with time-of-day and weather effects
- **ImGui Interface**: Customizable, dockable UI panels

## Development Phases

2. **Mini Metro Clone**: Implement a Minimum Viable Product (MVP) replicating core Mini Metro gameplay with adjustments to fit the game's systems
3. **Transport Mechanics**: Expand beyond simple lines to include distinct modes (road, rail initially), basic infrastructure placement (roads, tracks, stations/stops), and multi-modal pathfinding logic
4. **Deeper Simulation**: Enhance the 'cim' (citizen) simulation with individual needs, destinations (home, work, leisure based on zones), schedules, and more complex decision-making for route choices
5. **City Growth Mechanics**: Implement procedural generation of city expansion based on zoning, land value, accessibility, and population needs, allowing cities to evolve organically around the transport network
5. **Economy and External Factors**: Introduce core economic mechanics, including costs for building and maintaining infrastructure, vehicle running costs, ticket pricing, and revenue streams. Implement external simulation elements like random events (e.g., economic fluctuations, accidents, festivals), weather effects impacting operations, and potentially basic political or public satisfaction metrics influencing decisions
6. **Procedural Generation**: Develop the systems required to generate unique game worlds from scratch. This includes algorithms for terrain features (hills, water bodies), resource distribution, and potentially the initial placement and basic layout of procedurally generated towns or cities, offering endless replayability
7. **Real-world Data**: Integrate functionality to load and process real-world map data (likely from sources like OpenStreetMap). This allows players to select specific global locations, importing terrain heightmaps, water bodies, existing city locations, and possibly existing transit networks as a foundation for their transport empire
8. **Customization of systems**: Implement the options allowing players to tailor their gameplay experience by enabling or disabling specific simulation features
9. **UI refinement, assets, audio**: Focus on polishing the player experience. Make a custom user interface for better usability, clarity, and aesthetic appeal within the chosen art style. Expand the library of visual assets (vehicles, stations, track types, buildings) and implement sound effects and ambient audio/music to enhance immersion.
10. **Different game modes and onboarding**:  Develop and implement the distinct game modes outlined: a structured Campaign mode with goals, progression, and potentially AI competitors or government contracts, alongside the open-ended Free Play/Sandbox mode. Create tutorials, tooltips, or guided scenarios to effectively onboard new players to the game's potentially complex systems
11. **Modding API**: Design, implement, and document the official Modding Application Programming Interface (API). This empowers the community to extend the game by creating custom assets (vehicles, buildings, infrastructure), scripting new gameplay mechanics or events, tweaking simulation parameters, and potentially customizing UI elements
12. **Tweaking and polishing**: Conduct extensive playtesting and iteration based on feedback. Focus on balancing game mechanics (economy, cim behavior, transport efficiency), optimizing performance across different scenarios and hardware, fixing bugs, and refining the overall user experience to create a stable and enjoyable release candidate

## Technical Highlights

- **Entity-Component-System (ECS) Architecture**: Modular, data-oriented design for flexibility and performance.
- **Detailed Agent-Based Simulation**: Simulating individual 'cims' with unique needs, schedules, and complex route decision-making.
- **Multi-modal Pathfinding**: Optimized A* implementation with heuristics supporting diverse transport networks (road, rail, water, air).
- **Advanced Performance Optimization**: Leveraging spatial partitioning, extensive multithreading, and level-of-detail (LoD) systems for large-scale worlds.
- **Procedural World Generation**: Algorithms for creating unique and replayable game worlds from scratch.
- **Real-World Data Integration**: Capability to import and process geographic data (e.g., OpenStreetMap) to recreate real locations.
- **Extensible Modding API**: Powerful plugin system enabling community creation of assets, scripts, and gameplay modifications.
- **SFML/ImGui**: Utilized for clean rendering with effects (weather, time-of-day) and a flexible, dockable developer/user interface.

### Requirements
- Clang C++17
- CMake 3.31.7
- SFML 3.0.0 (documentation in docs folder)
- ImGui 1.91.9b
- ImGui-SFML 3.0
- EnTT 3.12.0

## License
[MIT License](LICENSE)