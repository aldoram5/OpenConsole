# OpenConsole Documentation

Welcome to the OpenConsole documentation! This guide will help you understand the architecture, develop new features, and contribute to the project.

## Table of Contents

### Getting Started
- [Project Overview](#project-overview)
- [Quick Start](development/GETTING_STARTED.md)
- [Building OpenConsole](development/BUILDING.md)

### Architecture
- [Architecture Overview](ARCHITECTURE.md)
- [Database Layer](architecture/DATABASE.md)
- [Plugin System](architecture/PLUGINS.md)
- [Game Launcher System](architecture/LAUNCHERS.md)

### API Reference
- [Database API](api/DATABASE_API.md)
- [Plugin API](api/PLUGIN_API.md)
- [Launcher API](api/LAUNCHER_API.md)

### Development Guides
- [Creating a Plugin](guides/CREATING_PLUGINS.md)
- [Adding a Game Launcher](guides/ADDING_LAUNCHERS.md)
- [Contributing Guidelines](development/CONTRIBUTING.md)

### Reference
- [Configuration](guides/CONFIGURATION.md)
- [Changelog](CHANGELOG.md)

## Project Overview

**OpenConsole** is a fork of EmulationStation redesigned as an open-source gaming console OS for modern indie games. It provides a controller-friendly interface for launching and managing games from multiple sources, with initial support for local games and itch.io.

### Key Features

- **Multi-Source Game Library**: Scan local directories, itch.io library, USB drives
- **Multiple Game Formats**: AppImage, Ren'Py, .deb packages, and more
- **SQLite Database**: Fast, reliable game metadata storage
- **Plugin Architecture**: Extensible system for adding new game sources
- **Game Launchers**: Type-specific launchers with process management
- **Controller-First UI**: Designed for living room gaming

### Technology Stack

- **Language**: C++11
- **Graphics**: SDL2 + OpenGL/OpenGL ES
- **Database**: SQLite3
- **Build System**: CMake
- **Platforms**: Linux (Raspberry Pi 5 primary target)

## Recent Changes (OpenConsole-Specific)

OpenConsole has added the following major features on top of EmulationStation:

### Phase 1: Foundation (Complete)
- SQLite3 database for game metadata
- Plugin system for extensible game sources
- Game launcher framework with AppImage, Ren'Py, and .deb support

### Phase 2: Local Game Support (Complete)
- Local filesystem scanner plugin
- Recursive directory scanning
- Automatic game type detection
- Cover art discovery
- Game name extraction and sanitization

### Phase 3: UI Components (In Progress)
- Virtual keyboard with controller navigation
- Game list UI integration
- Settings menus

### Phase 4: itch.io Integration (Planned)
- itch.io plugin with OAuth authentication
- Game library browsing
- Download manager

### Phase 5: System Integration (Planned)
- Systemd service
- Boot configuration
- Raspberry Pi OS image

## Quick Links

- [GitHub Repository](https://github.com/aldoram5/OpenConsole)
- [Original EmulationStation](https://github.com/RetroPie/EmulationStation)
- [Product Requirements Document](../PRD.md)

## Documentation Status

| Document | Status | Last Updated |
|----------|--------|--------------|
| Architecture Overview | ✅ Complete | 2025-11-11 |
| Database Layer | ✅ Complete | 2025-11-11 |
| Plugin System | ✅ Complete | 2025-11-11 |
| Launcher System | ✅ Complete | 2025-11-11 |
| Development Guide | ✅ Complete | 2025-11-11 |
| API Reference | ✅ Complete | 2025-11-11 |

## Getting Help

- **Issues**: [GitHub Issues](https://github.com/aldoram5/OpenConsole/issues)
- **Discussions**: [GitHub Discussions](https://github.com/aldoram5/OpenConsole/discussions)

## License

OpenConsole is licensed under the same license as EmulationStation. See [LICENSE.md](../LICENSE.md) for details.
