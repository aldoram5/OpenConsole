# OpenConsole Changelog

All notable changes to OpenConsole (fork-specific features) are documented in this file.

## [Unreleased]

### Phase 3 - UI Components (In Progress)
- Virtual keyboard with controller navigation
- Game list UI integration with database
- Settings menus for configuration

### Phase 4 - itch.io Integration (Planned)
- itch.io plugin with OAuth authentication
- Game library browsing and management
- Download manager with progress tracking
- Encrypted token storage

### Phase 5 - System Integration (Planned)
- Systemd service for auto-start
- Boot configuration for Raspberry Pi
- Pi OS image build scripts
- Controller configuration utilities

## [0.2.0] - 2025-11-11

### Phase 2: Local Game Support

#### Added
- **LocalFilesystemPlugin**: Complete local game scanner
  - Recursive directory scanning with depth limits
  - Configurable scan paths (~/Games, ~/.local/share/games, etc.)
  - Auto-detection of game types (AppImage, Ren'Py, .deb)
  - Automatic cover art discovery
  - Smart game name extraction and sanitization

- **GameScanner**: High-level scanning orchestration
  - Database population from scanned games
  - Progress callback support for UI integration
  - Scan statistics (found, added, skipped, errors)
  - Duplicate detection and handling
  - Database cleanup for missing games
  - Support for multiple scanning modes

#### Features
- Smart game name sanitization (removes version numbers, platform suffixes)
- Cover art auto-discovery (cover.*, poster.*, box.*, etc.)
- Duplicate prevention by executable path
- Performance tracking (scan duration)
- Comprehensive error handling and logging

#### Integration
- PluginManager auto-registers LocalFilesystemPlugin on init
- Updated CMakeLists.txt with new source files
- Ready for UI integration in Phase 3

#### Files Added
- `es-app/src/plugins/LocalFilesystemPlugin.h/cpp`
- `es-app/src/GameScanner.h/cpp`

## [0.1.0] - 2025-11-11

### Phase 1: Foundation

#### Added
- **Database Layer**: Complete SQLite3 integration
  - DatabaseManager singleton class
  - Schema for games and itch_games tables
  - CRUD operations for game metadata
  - Support for multiple game sources (local, itch.io, USB)
  - Play statistics tracking (last_played, play_count)
  - Indices for query performance
  - Prepared statements for efficiency

- **Plugin System**: Extensible game source architecture
  - IGameSourcePlugin interface
  - PluginManager singleton for lifecycle management
  - Authentication support with AuthResult
  - Download progress callbacks
  - Plugin registration and discovery

- **Game Launcher System**: Type-specific game execution
  - IGameLauncher interface
  - GameLauncherFactory with auto-detection
  - AppImageLauncher for .AppImage games
  - RenpyLauncher for Ren'Py visual novels
  - DebLauncher for .deb packages
  - Process management (fork/exec, monitoring, termination)
  - Validation and permission handling

#### Build System
- Added SQLite3 dependency to CMake
- Created FindSQLite3.cmake module
- Updated es-app/CMakeLists.txt with new sources
- Added OpenConsole namespace

#### Data Types
- GameType enum (APPIMAGE, RENPY, DEB, ELECTRON, DIRECTORY)
- GameSource enum (LOCAL, ITCH_IO, USB)
- GameMetadata struct with comprehensive fields
- ValidationResult and LaunchResult structures

#### Files Added
- `CMake/Packages/FindSQLite3.cmake`
- `es-app/src/db/DatabaseManager.h/cpp`
- `es-app/src/plugins/IGameSourcePlugin.h`
- `es-app/src/plugins/PluginManager.h/cpp`
- `es-app/src/launchers/IGameLauncher.h`
- `es-app/src/launchers/GameLauncherFactory.h/cpp`
- `es-app/src/launchers/AppImageLauncher.h/cpp`
- `es-app/src/launchers/RenpyLauncher.h/cpp`
- `es-app/src/launchers/DebLauncher.h/cpp`

## [0.0.1] - 2025-11-11

### Initial Fork
- Forked from RetroPie/EmulationStation
- Renamed project to OpenConsole
- Updated README with OpenConsole vision
- Created Product Requirements Document (PRD)
- Set up development branch structure

---

## EmulationStation Base

OpenConsole is built on top of EmulationStation. For EmulationStation's original changelog and history, see the [upstream repository](https://github.com/RetroPie/EmulationStation).

### Inherited Features

From EmulationStation we inherit:
- **UI Framework**: Component-based GUI system with SDL2 and OpenGL
- **Theme System**: XML-based theming with extensive customization
- **Input System**: Controller and keyboard support with remapping
- **Metadata Scrapers**: GamesDB and ScreenScraper integration
- **Collections**: Favorites, last played, custom collections
- **Views**: Multiple game list view types (basic, detailed, grid, video)
- **Platform Support**: Linux, Windows, Mac, Raspberry Pi

---

## Migration Notes

### From EmulationStation to OpenConsole

OpenConsole maintains compatibility with EmulationStation configurations while adding new features:

**Compatible**:
- Theme files (existing ES themes work)
- Controller configurations
- Most settings

**New/Different**:
- Game metadata now stored in SQLite (games.db) instead of gamelist.xml
- Plugin system for game sources (replaces system-based organization)
- Game type detection (replaces extension-based systems)

**Migration Path** (future):
- Tool to import existing gamelist.xml into SQLite database
- Automated backup of EmulationStation configs

---

## Version Scheme

OpenConsole uses semantic versioning: `MAJOR.MINOR.PATCH`

- **MAJOR**: Significant architectural changes
- **MINOR**: New features, phases completed
- **PATCH**: Bug fixes, minor improvements

Current phase: **Phase 2 Complete** (Local Game Support)

---

## Links

- **Repository**: https://github.com/aldoram5/OpenConsole
- **Issues**: https://github.com/aldoram5/OpenConsole/issues
- **Discussions**: https://github.com/aldoram5/OpenConsole/discussions
- **Documentation**: /docs/
