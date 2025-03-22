#!/bin/bash
# Linux setup script for Transity

# Detect package manager
if command -v apt-get &> /dev/null; then
    # Debian/Ubuntu
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        libsfml-dev \
        nlohmann-json3-dev \
        libspdlog-dev \
        libcatch2-dev \
        git
elif command -v dnf &> /dev/null; then
    # Fedora
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        ninja-build \
        SFML-devel \
        json-devel \
        spdlog-devel \
        catch-devel \
        git
elif command -v pacman &> /dev/null; then
    # Arch Linux
    sudo pacman -Syu --noconfirm \
        base-devel \
        cmake \
        ninja \
        sfml \
        nlohmann-json \
        spdlog \
        catch2 \
        git
else
    echo "Unsupported package manager. Please install dependencies manually."
    exit 1
fi

# Clone and build ImGui
if [ ! -d "external/imgui" ]; then
    mkdir -p external
    git clone https://github.com/ocornut/imgui.git external/imgui
    cd external/imgui
    
    # Create a simple CMakeLists.txt for ImGui
    cat > CMakeLists.txt << EOL
cmake_minimum_required(VERSION 3.20)
project(imgui)

add_library(imgui
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_sfml.cpp
)

target_include_directories(imgui PUBLIC .)
EOL

    # Build and install ImGui
    mkdir build && cd build
    cmake ..
    make
    sudo make install
    cd ../../..
fi

# Install EnTT (not available in all package managers)
if [ ! -d "external/entt" ]; then
    mkdir -p external
    git clone https://github.com/skypjack/entt.git external/entt
    cd external/entt
    mkdir build && cd build
    cmake ..
    make
    sudo make install
    cd ../../..
fi

echo "Setup complete! You can now build Transity." 