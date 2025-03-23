cmake_minimum_required(VERSION 3.20)

# Enable vcpkg manifest mode
set(VCPKG_MANIFEST_MODE ON)
set(VCPKG_MANIFEST_INSTALL ON)

project(Transity VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler-specific options
if(MSVC)
    # Add UTF-8 support for MSVC
    add_compile_options(/utf-8)
    # Disable warnings about unsafe CRT functions
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Enable testing
enable_testing()

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Dependencies
find_package(SFML CONFIG REQUIRED COMPONENTS system window graphics)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(Catch2 3 CONFIG REQUIRED)

# Define dependency interface library
add_library(transity_dependencies INTERFACE)
target_link_libraries(transity_dependencies INTERFACE
    sfml-system
    sfml-window
    sfml-graphics
    imgui::imgui
    ImGui-SFML::ImGui-SFML
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    EnTT::EnTT
    Catch2::Catch2
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)

# Install targets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION share/transity)

# Add preprocessor definitions for SFML
if(WIN32)
    add_definitions(-DSFML_STATIC)
endif() 