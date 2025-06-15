cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

# Where the final binaries will be placed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- pull SFML 2.6.1 from GitHub -------------------------------------------
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.0       # <- pinned to the last 2.x release line
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)
set(SFML_DIR ${SFML_BINARY_DIR})

# ---- pull EnTT from GitHub -------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.12.0  # Or the latest stable version
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(EnTT)

# ---- pull ImGui from GitHub --------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9b # Or a specific version tag like v1.89.9
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# ---- pull ImGui-SFML from GitHub ---------------------------------------------
include(FetchContent)
FetchContent_Declare(
    ImGuiSFML
    GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml.git
    GIT_TAG        v3.0 # Or the latest compatible version
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(ImGuiSFML)

# ---- your executable --------------------------------------------------------
add_executable(main
    src/main.cpp
    src/Game.cpp
    src/core/Camera.cpp
    src/input/InputHandler.cpp
    src/graphics/Renderer.cpp
    src/core/EntityFactory.cpp
    src/Logger.cpp
    src/graphics/UI.cpp
    src/systems/LineCreationSystem.cpp
    src/world/WorldGenerationSystem.cpp
    src/world/TerrainRenderSystem.cpp
)
target_compile_features(main PRIVATE cxx_std_17)

# Add include directory
target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${entt_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}          # For ImGui headers
    ${imgui_SOURCE_DIR}/backends # For ImGui backend headers (if needed directly)
    ${ImGuiSFML_SOURCE_DIR}      # For ImGui-SFML headers
)

target_link_libraries(main PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML
)
