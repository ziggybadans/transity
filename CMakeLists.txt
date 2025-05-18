cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

# Where the final binaries will be placed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ---- pull SFML 2.6.1 from GitHub -------------------------------------------
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.1       # <- pinned to the last 2.x release line
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# ---- pull EnTT from GitHub -------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.0  # Or the latest stable version
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(EnTT)

# ---- your executable --------------------------------------------------------
add_executable(main
    src/main.cpp
    src/Game.cpp
    src/Camera.cpp
    src/InputHandler.cpp
    src/Renderer.cpp
)
target_compile_features(main PRIVATE cxx_std_17)

# Add include directory
target_include_directories(main PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${entt_SOURCE_DIR}/src
)

# SFML 2.x exports the old, lower-case targets
target_link_libraries(main PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)
