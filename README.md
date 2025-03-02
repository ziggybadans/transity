# Transity

A game project that simulates transportation systems.

## Overview

Transity is a simulation game focused on building and managing transportation networks. Players can design and optimize various transportation systems to move people and goods efficiently.

## Features

- Build and manage transportation networks
- Simulate passenger and cargo movement
- Optimize routes for efficiency
- Manage resources and budget

## Development Setup

### Prerequisites

#### Windows

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with C++ development tools
2. Install [CMake](https://cmake.org/download/) (3.14 or higher)
3. Install [Git](https://git-scm.com/download/win)
4. Install [SFML](https://www.sfml-dev.org/download.php) (2.5 or higher)
5. Install [VS Code](https://code.visualstudio.com/) (optional, but recommended)

#### macOS

1. Install [Xcode](https://apps.apple.com/us/app/xcode/id497799835) or Command Line Tools (`xcode-select --install`)
2. Install [Homebrew](https://brew.sh/)
3. Install required packages:
   ```bash
   brew install cmake ninja sfml
   ```
4. Install [VS Code](https://code.visualstudio.com/) (optional, but recommended)

### VS Code Extensions

Install the following VS Code extensions for the best development experience:

- C/C++ Extension Pack
- CMake Tools
- CMake Language Support
- Code Runner

### Building the Project

#### Using VS Code

1. Open the project folder in VS Code
2. Select a kit when prompted by CMake Tools
3. Configure and build the project using the CMake Tools extension
4. Run the project using the "Run" task or the debug launch configuration

#### Using Command Line (macOS and Windows)

```bash
# Configure the project
cmake -B build -S .

# Build the project
cmake --build build

# Run the project
./build/bin/Transity  # macOS
.\build\bin\Debug\Transity.exe  # Windows
```

## Project Structure

- `src/`: Source code
- `include/`: External libraries and headers
- `assets/`: Game assets (graphics, sounds, etc.)
- `docs/`: Documentation
- `tests/`: Unit and integration tests
- `config/`: Configuration files

## Development Workflow

This project follows a structured development workflow as outlined in the [contributing guidelines](.github/contributing.md).

### Branch Strategy

- `main`: Production-ready code
- `staging`: Pre-production testing
- `develop`: Active development branch

### Getting Started

1. Clone the repository
2. Create a feature branch from the appropriate release branch
3. Make your changes
4. Submit a pull request

## License

[License information to be added]
