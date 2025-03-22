# Transity

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A transport management simulator with elements of a city builder, featuring a simple art style with complex simulation mechanics.

## Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler
- Ninja build system
- SFML 2.5 or higher
- ImGui
- nlohmann_json 3.11.2 or higher
- spdlog
- EnTT
- Catch2 v3 (for testing)

## Building

### Windows

1. Install Visual Studio 2022 with C++ workload
2. Run the setup script as administrator:
   ```powershell
   .\tools\setup_windows.ps1
   ```
3. Open the project in Visual Studio Code
4. Select the Visual Studio 2022 kit when prompted by CMake Tools
5. Click Build in the status bar or press F7

### macOS

1. Run the setup script:
   ```bash
   chmod +x tools/setup_macos.sh
   ./tools/setup_macos.sh
   ```
2. Open the project in Visual Studio Code
3. Select the Clang kit when prompted by CMake Tools
4. Click Build in the status bar or press F7

### Linux

1. Run the setup script:
   ```bash
   chmod +x tools/setup_linux.sh
   ./tools/setup_linux.sh
   ```
2. Open the project in Visual Studio Code
3. Select the GCC kit when prompted by CMake Tools
4. Click Build in the status bar or press F7

## Project Structure

```
transity/
├── assets/              # Game assets (textures, fonts, sounds)
├── cmake/               # CMake modules and configuration
├── docs/                # Documentation
├── include/             # Public headers
├── src/                 # Source files
├── tests/               # Test files
└── tools/               # Development and build tools
```

## Development

- Build types:
  - Debug (default): Includes debug symbols and assertions
  - Release: Optimized build for distribution
  - RelWithDebInfo: Release with debug information
  - MinSizeRel: Minimal size release build

- Running tests:
  ```bash
  cd build
  ctest --output-on-failure
  ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.