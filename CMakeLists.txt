cmake_minimum_required(VERSION 3.20)
project(Transity VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)
find_package(ImGui REQUIRED)
find_package(nlohmann_json 3.11.2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(EnTT REQUIRED)
find_package(Catch2 3 REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Install targets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION share/transity) 