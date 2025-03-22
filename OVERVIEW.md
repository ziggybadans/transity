# Transity
## Overview
Transity is a transport management simulator with elements of a city builder. Unlike games like Cities Skylines and Transport Fever, it is not meant to have realistic graphics, but instead draw players in with a simple, colourful art style combined with a complex, accurate simulation. Transity is inspired by games like Mini Metro and NIMBY Rails but with a desire for the complexity and replayability of Cities Skylines and Transport Fever.

The game allows players to plan infrastructure like roads, railways, waterways and airways, create routes using different modes of transport (eg. bus, tram, train, plane, ferry, etc.) and then simulate those routes. While the city's buildings can't be plopped directly like in regular city builders, areas can be zoned and suited to different purposes.

One of the core parts of Transity is endless customisation. You can choose to play in a procedurally generated random world, or choose a place in the world you know, pulled from map data. You can play in a campaign mode where you start one or multiple transport companies and work with the government and public to improve infrastructure, or in a free play mode where you can just build anything you want. You can also turn off any simulation features you don't like (eg. politics, disasters, manual placement of required infrastructure, etc.)

If further customisation is needed, there is a modding API that allows mods to be created covering any part of the game, most notably adding new assets (eg. trains).

## Technical Info
- GUI is made with ImGui
- Game is coded in C++ with SFML
- Supports all devices, including Windows/Mac and touchscreen devices
- Modular code utilising all modern principles, including ECS
- Performant and optimised, utilising higher-end hardware to allow the choice of either more complex simulation or a faster simulation speed
- A very large map size that is dynamically loaded, with low-demand simulation estimations occurring in unloaded areas
- As allowed by the simple art style, dynamically generated assets are used where applicable, reducing the game's install size