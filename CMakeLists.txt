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

## EnTT
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG v3.15.0
)
FetchContent_MakeAvailable(entt)

## ImGui
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(ImGui)

## SFML
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.0
  CMAKE_ARGS
      -DBUILD_SHARED_LIBS=OFF
      -DSFML_USE_STATIC_STD_LIBS=ON
)
FetchContent_MakeAvailable(SFML)
FetchContent_GetProperties(SFML BINARY_DIR sfml_BINARY_DIR)
list(APPEND CMAKE_PREFIX_PATH "${sfml_BINARY_DIR}")

## ImGui-SFML
FetchContent_Declare(
  ImGui-SFML
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG v3.0
  IMGUI_SFML_FIND_SFML_STATIC=ON
  UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(ImGui-SFML)

# Subdirectories
add_subdirectory(src)

# Formatting
if(MSVC)
  # Add the /analyze flag to the C++ compiler flags for MSVC
  add_compile_options(/analyze)
  # Optionally, treat analysis warnings as errors (recommended for CI)
  # add_compile_options(/analyze:WX-) # Use /analyze:WX- to treat warnings as errors
  message(STATUS "MSVC /analyze static analysis enabled.")
endif()