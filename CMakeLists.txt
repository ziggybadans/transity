cmake_minimum_required(VERSION 3.28)
project(Transity VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "")

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

# ---- pull nlohmann/json from GitHub ----------------------------------------
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2 # Or a recent stable version
    GIT_SHALLOW    ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(nlohmann_json)

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

set(TRANSITY_STAGE "Predev")

# ---- your executable --------------------------------------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")

if(APPLE)
    add_executable(transity MACOSX_BUNDLE ${SOURCES})
elseif(WIN32)
    add_executable(transity WIN32 ${SOURCES})
else()
    add_executable(transity ${SOURCES})
endif()

target_compile_definitions(transity PRIVATE
    LOGGING_ENABLED
    TRANSITY_VERSION="${PROJECT_VERSION}"
    TRANSITY_STAGE="${TRANSITY_STAGE}"
)
target_compile_features(transity PRIVATE cxx_std_17)

# Add include directory
target_include_directories(transity PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${entt_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}          # For ImGui headers
    ${imgui_SOURCE_DIR}/backends # For ImGui backend headers (if needed directly)
    ${ImGuiSFML_SOURCE_DIR}      # For ImGui-SFML headers
    ${nlohmann_json_SOURCE_DIR}/include
)

target_link_libraries(transity PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML
)

set_target_properties(transity PROPERTIES OUTPUT_NAME "Transity")

add_custom_command(TARGET transity POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:transity>/data"
    COMMENT "Copying data directory to build output"
)
