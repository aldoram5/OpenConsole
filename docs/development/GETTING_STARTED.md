# Getting Started with OpenConsole Development

This guide will help you set up your development environment and start contributing to OpenConsole.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Getting the Source](#getting-the-source)
3. [Building OpenConsole](#building-openconsole)
4. [Project Structure](#project-structure)
5. [Development Workflow](#development-workflow)
6. [Running and Testing](#running-and-testing)
7. [Common Tasks](#common-tasks)

## Prerequisites

### Required Tools

- **C++ Compiler**: GCC 4.7+ or Clang with C++11 support
- **CMake**: 3.5 or higher
- **Git**: For version control

### Required Libraries

```bash
# Ubuntu/Debian
sudo apt-get install \
    build-essential \
    cmake \
    git \
    libsdl2-dev \
    libfreeimage-dev \
    libfreetype6-dev \
    libcurl4-openssl-dev \
    libasound2-dev \
    libgl1-mesa-dev \
    libvlc-dev \
    libsqlite3-dev \
    rapidjson-dev

# Raspberry Pi (additional)
sudo apt-get install \
    libraspberrypi-dev

# Optional: libCEC for TV control
sudo apt-get install libcec-dev
```

### Development Tools (Recommended)

```bash
# Code editor (choose one)
sudo apt-get install vim
# or
sudo snap install code --classic  # VS Code

# Debugging tools
sudo apt-get install gdb valgrind

# Documentation
sudo apt-get install doxygen graphviz
```

## Getting the Source

### Clone the Repository

```bash
git clone https://github.com/aldoram5/OpenConsole.git
cd OpenConsole
```

### Initialize Submodules

```bash
git submodule update --init --recursive
```

This fetches:
- `external/pugixml` - XML parsing
- `external/nanosvg` - SVG rendering

## Building OpenConsole

### Quick Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build (use -j for parallel compilation)
make -j$(nproc)

# Binary will be in project root
ls ../emulationstation
```

### Build Configurations

**Debug Build** (with symbols, no optimization):
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

**Release Build** (optimized):
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Raspberry Pi Build**:
```bash
cmake .. -DRPI=ON -DCMAKE_BUILD_TYPE=Release
make -j4
```

### Build Options

```bash
# Desktop OpenGL 2.1
cmake .. -DUSE_GL21=ON

# Embedded OpenGL ES 2.0
cmake .. -DGLES=ON

# Enable CEC support
cmake .. -DCEC=ON

# Enable profiling
cmake .. -DPROFILING=ON
```

## Project Structure

```
OpenConsole/
├── CMakeLists.txt              # Root CMake configuration
├── es-core/                    # Core rendering and UI system
│   ├── src/
│   │   ├── components/         # UI components
│   │   ├── guis/              # Core GUI dialogs
│   │   ├── renderers/         # OpenGL renderers
│   │   ├── resources/         # Resource management
│   │   └── utils/             # Utility functions
│   └── CMakeLists.txt
├── es-app/                     # Application layer
│   ├── src/
│   │   ├── components/        # App-specific components
│   │   ├── guis/              # Application dialogs
│   │   ├── views/             # Game list views
│   │   ├── scrapers/          # Metadata scrapers
│   │   ├── db/                # [NEW] Database layer
│   │   ├── plugins/           # [NEW] Plugin system
│   │   ├── launchers/         # [NEW] Game launchers
│   │   └── main.cpp           # Application entry point
│   └── CMakeLists.txt
├── external/                   # Third-party libraries
│   ├── pugixml/
│   └── nanosvg/
├── resources/                  # Built-in assets
├── themes/                     # Default themes
├── docs/                       # [NEW] Documentation
└── CMake/                      # CMake modules
    └── Packages/               # Find* modules
```

## Development Workflow

### 1. Create a Feature Branch

```bash
git checkout -b feature/my-new-feature
```

### 2. Make Changes

Edit source files in `es-app/src/` or `es-core/src/`

### 3. Rebuild

```bash
cd build
make -j$(nproc)
```

Incremental builds are fast - only changed files recompile.

### 4. Test

```bash
# Run from project root
./emulationstation --debug
```

### 5. Commit Changes

```bash
git add .
git commit -m "Add new feature: description"
```

### 6. Push and Create PR

```bash
git push origin feature/my-new-feature
# Create pull request on GitHub
```

## Running and Testing

### Running OpenConsole

```bash
# Normal run
./emulationstation

# Debug mode (verbose logging)
./emulationstation --debug

# Windowed mode (for development)
./emulationstation --windowed

# Set home directory
./emulationstation --home ~/.openconsole
```

### Command Line Options

```bash
--home <path>          # Set config directory
--resolution <w> <h>   # Set window resolution
--windowed             # Run in window (not fullscreen)
--debug                # Enable debug logging
--no-exit              # Don't allow quitting
--help                 # Show all options
```

### Viewing Logs

```bash
# Log location
tail -f ~/.emulationstation/es_log.txt

# Filter for errors
grep "Error" ~/.emulationstation/es_log.txt

# Filter for specific component
grep "DatabaseManager" ~/.emulationstation/es_log.txt
```

### Using GDB

```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run with GDB
gdb ./emulationstation

# Inside GDB
(gdb) run --debug
(gdb) bt              # Backtrace on crash
(gdb) break main      # Set breakpoint
(gdb) continue        # Continue execution
```

### Memory Debugging with Valgrind

```bash
valgrind --leak-check=full ./emulationstation --debug
```

## Common Tasks

### Adding a New Source File

1. Create `MyNewClass.h` and `MyNewClass.cpp` in appropriate directory
2. Add to `es-app/CMakeLists.txt`:

```cmake
set(ES_HEADERS
    # ... existing headers ...
    ${CMAKE_CURRENT_SOURCE_DIR}/src/path/MyNewClass.h
)

set(ES_SOURCES
    # ... existing sources ...
    ${CMAKE_CURRENT_SOURCE_DIR}/src/path/MyNewClass.cpp
)
```

3. Rebuild:
```bash
cd build
cmake ..  # Regenerate build files
make -j$(nproc)
```

### Adding a New Dependency

1. Check if Find module exists in `CMake/Packages/`
2. If not, create `FindMyLib.cmake`
3. Add to root `CMakeLists.txt`:

```cmake
find_package(MyLib REQUIRED)

LIST(APPEND COMMON_INCLUDE_DIRS
    ${MyLib_INCLUDE_DIRS}
)

set(COMMON_LIBRARIES
    # ... existing libs ...
    ${MyLib_LIBRARIES}
)
```

### Adding Log Statements

```cpp
#include "Log.h"

LOG(LogError) << "Critical error occurred";
LOG(LogWarning) << "Warning: something unexpected";
LOG(LogInfo) << "Information message";
LOG(LogDebug) << "Debug details";  // Only with --debug
```

### Creating a New Plugin

See [Creating Plugins Guide](../guides/CREATING_PLUGINS.md)

### Creating a New Launcher

See [Adding Launchers Guide](../guides/ADDING_LAUNCHERS.md)

### Modifying the Database Schema

1. Update `DatabaseManager::createSchema()` in `es-app/src/db/DatabaseManager.cpp`
2. Add migration logic (future enhancement)
3. Test with fresh database:

```bash
rm ~/.openconsole/games.db
./emulationstation --debug
```

## Code Style

### Naming Conventions

- **Classes**: PascalCase (`MyClass`)
- **Functions**: camelCase (`myFunction`)
- **Variables**: camelCase (`myVariable`)
- **Members**: mPrefix (`mMyMember`)
- **Constants**: UPPER_CASE (`MY_CONSTANT`)

### Example

```cpp
class GameScanner {
public:
    GameScanner();

    ScanStats scanAllSources();
    void setSkipDuplicates(bool skip);

private:
    bool mSkipDuplicates;
    std::string mLastError;

    static const int MAX_SCAN_DEPTH = 5;
};
```

### Header Guards

```cpp
#pragma once
#ifndef ES_APP_MY_CLASS_H
#define ES_APP_MY_CLASS_H

// ... class definition ...

#endif // ES_APP_MY_CLASS_H
```

### Includes

```cpp
// System includes first
#include <string>
#include <vector>

// Third-party includes
#include <SDL.h>

// Project includes
#include "Log.h"
#include "MyClass.h"
```

## IDE Setup

### VS Code

Recommended extensions:
- C/C++ (Microsoft)
- CMake Tools
- GitLens

`.vscode/settings.json`:
```json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "files.associations": {
        "*.h": "cpp"
    }
}
```

### Vim/Neovim

Install:
- YouCompleteMe or coc.nvim for C++ completion
- vim-cmake for CMake integration

`.vimrc`:
```vim
set cindent
set tabstop=4
set shiftwidth=4
set noexpandtab
```

## Debugging Tips

### Print Debugging

```cpp
LOG(LogDebug) << "Variable value: " << myVar;
LOG(LogDebug) << "Entering function: " << __FUNCTION__;
```

### Breakpoints

```cpp
#include <signal.h>

// Trigger breakpoint
raise(SIGTRAP);
```

### Conditional Compilation

```cpp
#ifdef _DEBUG
    LOG(LogDebug) << "Debug-only message";
#endif
```

## Performance Profiling

### Using gprof

```bash
# Build with profiling
cmake .. -DPROFILING=ON
make -j$(nproc)

# Run to generate profile data
./emulationstation

# Analyze
gprof ./emulationstation gmon.out > analysis.txt
```

### Using perf (Linux)

```bash
# Record
perf record ./emulationstation

# Report
perf report
```

## Continuous Integration

OpenConsole uses GitHub Actions for CI. Check `.github/workflows/` for configuration.

Local CI testing:
```bash
# Run build in clean environment
docker run -v $(pwd):/openconsole -w /openconsole ubuntu:20.04 bash -c "
    apt-get update &&
    apt-get install -y build-essential cmake ... &&
    mkdir build && cd build &&
    cmake .. &&
    make -j
"
```

## Getting Help

- **Documentation**: `docs/` directory
- **Issues**: [GitHub Issues](https://github.com/aldoram5/OpenConsole/issues)
- **Discussions**: [GitHub Discussions](https://github.com/aldoram5/OpenConsole/discussions)
- **Original ES Docs**: `/docs` in this repo (inherited from EmulationStation)

## Next Steps

Now that you have OpenConsole building:

1. Read [Architecture Overview](../ARCHITECTURE.md)
2. Explore [Plugin System](../architecture/PLUGINS.md)
3. Check out [API Reference](../api/)
4. Try [Creating a Plugin](../guides/CREATING_PLUGINS.md)

Happy coding! 🎮
