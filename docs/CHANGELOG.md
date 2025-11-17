# OpenConsole Changelog

All notable changes to OpenConsole (fork-specific features) are documented in this file.

## [Unreleased]

### Phase 5 - System Integration (Planned)
- Systemd service for auto-start
- Boot configuration for Raspberry Pi
- Pi OS image build scripts
- Controller configuration utilities

## [0.4.0] - 2025-11-14

### Phase 4: itch.io Integration

#### Added
- **TokenStorage**: Secure credential storage system
  - AES-256-CBC encryption for API tokens
  - Machine-specific encryption key derivation using SHA-256
  - Stores encrypted credentials in `~/.openconsole/credentials.enc`
  - File permissions set to 0600 for security
  - JSON-based storage format with RapidJSON
  - Singleton pattern for credential management
  - Methods: storeToken(), getToken(), hasToken(), removeToken(), clearAll()

- **ItchIoApiClient**: HTTP client for itch.io API
  - Complete API integration with itch.io endpoints
  - **Endpoints**:
    - GET /profile - Fetch user profile information
    - GET /profile/owned-keys - Retrieve owned games library
    - GET /uploads/{id}/download - Get temporary download URLs
  - CURL-based implementation with SSL support
  - Download progress callbacks for UI integration
  - Automatic redirect following and retry logic
  - JSON response parsing with error handling
  - User agent: "OpenConsole/1.0"

- **ItchIoPlugin**: itch.io game source plugin
  - Implements IGameSourcePlugin interface
  - API key based authentication (no OAuth required)
  - Fetches owned games from user's itch.io library
  - Download games with real-time progress tracking
  - Auto-detection of game types from filenames:
    - AppImage (.appimage)
    - Ren'Py (renpy in filename)
    - .deb packages (.deb)
    - Electron (electron in filename)
    - Archives (.zip, .tar)
  - Secure credential storage via TokenStorage
  - Download location: `~/.openconsole/downloads/itch.io/{game_id}/`
  - Registered automatically in PluginManager

- **GuiItchIoAuth**: Authentication dialog for itch.io
  - Controller-first UI for API key entry
  - Virtual keyboard integration for text input
  - Real-time API key validation and testing
  - User instructions for obtaining API keys
  - Display authentication status (authenticated/not authenticated)
  - Show authenticated username and user ID
  - Logout functionality to clear credentials
  - Callback system for authentication completion

#### Integration
- **GuiOpenConsoleSettings** extended with itch.io section:
  - itch.io authentication status display
    - Green "Authenticated" when logged in
    - Red "Not authenticated" when logged out
  - "itch.io Authentication" → "CONFIGURE" button
  - Opens GuiItchIoAuth dialog for credential management
  - Success message on successful authentication
- **PluginManager** auto-registers ItchIoPlugin on initialization
- **GameScanner** includes itch.io games when scanning
- Updated CMakeLists.txt with all new Phase 4 files

#### Security Features
- AES-256-CBC encryption for stored API keys
- Machine-specific encryption keys (non-portable)
- SHA-256 key derivation from hostname + home path + salt
- Restrictive file permissions (0600) on credentials file
- No plaintext credentials in logs or memory after encryption
- Secure random IV generation for each encryption operation
- OpenSSL-based cryptography

#### Technical Details
- **Dependencies**:
  - OpenSSL (libcrypto) for AES encryption
  - libCURL for HTTP requests
  - RapidJSON for API response parsing
  - Boost.Filesystem for path handling
- **Architecture**:
  - TokenStorage: Singleton pattern
  - ItchIoApiClient: HTTP client layer
  - ItchIoPlugin: Plugin layer (IGameSourcePlugin)
  - GuiItchIoAuth: UI layer (GuiComponent)
- **Error Handling**:
  - Comprehensive error messages from API
  - Failed authentication shows specific error reasons
  - Network errors handled with CURL error codes
  - JSON parsing errors caught and logged

#### User Workflow
1. Open OpenConsole Settings → Plugins section
2. See "itch.io Status: Not authenticated" (red)
3. Select "itch.io Authentication" → "CONFIGURE"
4. View instructions to visit itch.io/user/settings/api-keys
5. Generate API key on itch.io website
6. Enter API key using virtual keyboard
7. System tests key and authenticates automatically
8. Success message displays: "Authenticated as: {username}"
9. Status updates to green "Authenticated"
10. Scan for games to include itch.io library

#### Files Added
- `es-app/src/utils/TokenStorage.h/cpp`
- `es-app/src/api/ItchIoApiClient.h/cpp`
- `es-app/src/plugins/ItchIoPlugin.h/cpp`
- `es-app/src/guis/GuiItchIoAuth.h/cpp`

#### Files Modified
- `es-app/src/plugins/PluginManager.cpp` (registered ItchIoPlugin)
- `es-app/src/guis/GuiOpenConsoleSettings.h/cpp` (added itch.io section)
- `es-app/CMakeLists.txt` (added new files to build)

#### API Documentation
- itch.io API: https://itch.io/docs/api/overview
- API Key Generation: https://itch.io/user/settings/api-keys

## [0.3.0] - 2025-11-13

### Phase 3: UI Components and Database Integration

#### Added
- **GuiVirtualKeyboard**: On-screen keyboard for controller-based text input
  - QWERTY layout with full alphanumeric support
  - Shift support for uppercase letters and special characters
  - D-pad navigation optimized for living room gaming
  - Controller button mappings:
    - A (confirm): Select character
    - B (back): Backspace/delete
    - X: Insert space
    - Y: Toggle shift
    - Start: Accept and submit text
  - Callback system for text completion events
  - Multi-line text input support
  - Configurable initial text and accept button labels

- **GuiOpenConsoleSettings**: Comprehensive settings and configuration menu
  - **Database Section**:
    - Display database file path
    - View database statistics (game counts by type and source)
  - **Plugin Section**:
    - Show installed plugin count
    - Toggle recursive directory scanning
    - Configure maximum scan depth
    - Add/remove scan paths with virtual keyboard integration
  - **Scanning Section**:
    - Manual "Scan for Games Now" button
    - Toggle auto-scan on startup
    - Progress feedback during scanning operations
  - **Maintenance Section**:
    - Refresh database (rescan all sources)
    - Clear database (with confirmation dialog)
  - Integration with GameScanner for all scanning operations
  - Progress callbacks for real-time UI updates

- **OpenConsoleSystem**: Database-to-GameList bridge
  - Integrates GameMetadata from database with EmulationStation's FileData system
  - Converts database entries to FileData objects for display in game lists
  - System initialization and cleanup methods (init/deinit)
  - Game list refresh capability
  - Handles missing game files gracefully (logs warning, continues)
  - Stores database ID in FileData metadata for easy lookups
  - Creates virtual "OpenConsole" system for database-driven games

#### Integration
- Added "OPENCONSOLE SETTINGS" menu entry to main GuiMenu
  - Positioned at top of menu for easy access
  - Green color highlight (0x00FF00FF) for visibility
  - Opens GuiOpenConsoleSettings on selection
- Updated es-core/CMakeLists.txt with GuiVirtualKeyboard sources
- Updated es-app/CMakeLists.txt with OpenConsoleSystem and GuiOpenConsoleSettings sources

#### Technical Details
- Virtual keyboard uses ComponentGrid for structured layout
- Settings menu inherits from GuiSettings for consistent UI patterns
- OpenConsoleSystem acts as adapter between database layer and view layer
- All UI components follow EmulationStation's theme system conventions
- Controller-first design throughout (no mouse/keyboard required)

#### Files Added
- `es-core/src/guis/GuiVirtualKeyboard.h/cpp`
- `es-app/src/OpenConsoleSystem.h/cpp`
- `es-app/src/guis/GuiOpenConsoleSettings.h/cpp`

#### Files Modified
- `es-app/src/guis/GuiMenu.h/cpp` (added OpenConsole settings entry)
- `es-core/CMakeLists.txt` (added GuiVirtualKeyboard)
- `es-app/CMakeLists.txt` (added OpenConsoleSystem and GuiOpenConsoleSettings)

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

Current phase: **Phase 4 Complete** (itch.io Integration)

---

## Links

- **Repository**: https://github.com/aldoram5/OpenConsole
- **Issues**: https://github.com/aldoram5/OpenConsole/issues
- **Discussions**: https://github.com/aldoram5/OpenConsole/discussions
- **Documentation**: /docs/
