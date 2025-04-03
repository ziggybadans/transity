# Specify minimum version of CMake
cmake_minimum_required(VERSION 3.15)

# Declare project
project(Transity VERSION 0.1 LANGUAGES CXX)

# Includes
include(FetchContent)

# Project Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

## Google Test
#enable_testing()
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/googletest)

## EnTT
add_subdirectory(lib/entt)

## ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
add_library(imgui STATIC
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC lib/imgui)

## SFML
list(APPEND CMAKE_PREFIX_PATH "lib/SFML-2.6.1")
find_package(SFML 2.6.1 REQUIRED COMPONENTS system window graphics network audio)

## ImGui-SFML
add_subdirectory(lib/imgui-SFML)

# Subdirectories
add_subdirectory(src)
#add_subdirectory(tests)