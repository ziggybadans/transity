#!/bin/bash
# macOS setup script for Transity

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# Install dependencies using Homebrew
brew install \
    cmake \
    ninja \
    sfml \
    nlohmann-json \
    spdlog \
    entt \
    catch2

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

echo "Setup complete! You can now build Transity." 