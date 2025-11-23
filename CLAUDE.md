# OpenConsole Developer Guide for Claude

This document provides a comprehensive guide to understanding and navigating the OpenConsole codebase. It's designed to help AI assistants (and developers) quickly understand the project structure, locate key components, and make informed changes.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Codebase Structure](#codebase-structure)
3. [Key Components](#key-components)
4. [Recent Fixes & Improvements](#recent-fixes--improvements)
5. [Common Development Tasks](#common-development-tasks)
6. [Architecture Quick Reference](#architecture-quick-reference)
7. [Testing & Debugging](#testing--debugging)
8. [Important Files](#important-files)
9. [Development Workflow](#development-workflow)

---

## Project Overview

**OpenConsole** is a gaming console OS forked from EmulationStation, designed for modern indie games. It provides a controller-first interface for launching and managing games from multiple sources.

### Core Characteristics

- **Base**: Fork of EmulationStation (RetroArch/RetroPie)
- **Language**: C++11
- **Build System**: CMake
- **Primary Target**: Raspberry Pi 5 (also supports Pi 4, x86_64 Linux)
- **UI Framework**: Custom component-based system built on SDL2 + OpenGL
- **Database**: SQLite3 for game metadata
- **Key Features**: Local game scanning, built-in itch.io integration, controller-first UX

### What Makes OpenConsole Different from EmulationStation

1. **Database-Driven**: Uses SQLite instead of XML gamelists
2. **Plugin System**: Extensible game source plugins (local filesystem, itch.io)
3. **Modern Game Formats**: AppImage, Ren'Py, .deb, Electron apps (not just emulators)
4. **itch.io Integration**: Built-in support for browsing and downloading itch.io games
5. **Secure Storage**: AES-256 encrypted credential storage
6. **Virtual Keyboard**: On-screen QWERTY keyboard for controller-only text input

---

## Codebase Structure

```
OpenConsole/
├── es-core/                    # Core EmulationStation components (inherited)
│   ├── src/
│   │   ├── components/         # Base UI components (ImageComponent, TextComponent, etc.)
│   │   ├── guis/               # Core GUI screens (GuiMsgBox, GuiTextEditPopup, etc.)
│   │   ├── renderers/          # OpenGL rendering backends
│   │   ├── resources/          # Resource management (fonts, textures, sounds)
│   │   ├── utils/              # Core utilities (FileSystemUtil, StringUtil, etc.)
│   │   ├── animations/         # Animation system
│   │   ├── math/               # Vector, Transform classes
│   │   ├── Window.cpp          # Main window and GUI stack manager
│   │   ├── InputManager.cpp    # Input device handling
│   │   └── ThemeData.cpp       # Theme system
│   └── CMakeLists.txt
│
├── es-app/                     # Application-specific code (OpenConsole extensions)
│   ├── src/
│   │   ├── components/         # App-specific components (RatingComponent, etc.)
│   │   ├── guis/               # OpenConsole GUI screens ⭐
│   │   │   ├── GuiOpenConsoleSettings.cpp  # Main settings menu
│   │   │   ├── GuiVirtualKeyboard.cpp      # Virtual keyboard
│   │   │   ├── GuiItchIoAuth.cpp           # itch.io authentication dialog
│   │   │   ├── GuiMenu.cpp                 # Main menu
│   │   │   └── GuiSettings.cpp             # Base settings class
│   │   ├── db/                 # Database layer ⭐
│   │   │   ├── DatabaseManager.cpp         # SQLite database operations
│   │   │   └── DatabaseManager.h
│   │   ├── plugins/            # Game source plugins ⭐
│   │   │   ├── IGameSourcePlugin.h         # Plugin interface
│   │   │   ├── LocalFilesystemPlugin.cpp   # Local game scanning
│   │   │   ├── ItchIoPlugin.cpp            # itch.io integration
│   │   │   ├── PluginManager.cpp           # Plugin lifecycle management
│   │   │   └── GameScanner.cpp             # Orchestrates scanning
│   │   ├── launchers/          # Game launchers ⭐
│   │   │   ├── IGameLauncher.h             # Launcher interface
│   │   │   ├── AppImageLauncher.cpp        # Launch AppImage games
│   │   │   ├── RenpyLauncher.cpp           # Launch Ren'Py games
│   │   │   ├── DebLauncher.cpp             # Launch .deb packages
│   │   │   └── GameLauncherFactory.cpp     # Factory pattern for launchers
│   │   ├── api/                # External API clients ⭐
│   │   │   ├── ItchIoApiClient.cpp         # itch.io HTTP client
│   │   │   └── ItchIoApiClient.h
│   │   ├── utils/              # OpenConsole utilities ⭐
│   │   │   ├── TokenStorage.cpp            # Encrypted credential storage
│   │   │   └── TokenStorage.h
│   │   ├── views/              # View layer
│   │   │   ├── ViewController.cpp          # Main view controller
│   │   │   ├── SystemView.cpp              # System carousel view
│   │   │   └── gamelist/                   # Game list views
│   │   ├── scrapers/           # Metadata scrapers (inherited)
│   │   ├── animations/         # App-specific animations
│   │   ├── OpenConsoleSystem.cpp  # Database-to-UI bridge ⭐
│   │   ├── SystemData.cpp      # System/collection data
│   │   ├── FileData.cpp        # Game file data
│   │   └── main.cpp            # Application entry point
│   └── CMakeLists.txt
│
├── docs/                       # Documentation
│   ├── ARCHITECTURE.md         # High-level architecture overview
│   ├── architecture/           # Detailed architecture docs
│   │   ├── DATABASE.md         # Database layer details
│   │   ├── PLUGINS.md          # Plugin system details
│   │   └── LAUNCHERS.md        # Launcher system details
│   ├── guides/                 # User and developer guides
│   │   ├── UI_COMPONENTS.md    # UI component usage
│   │   ├── ITCHIO_INTEGRATION.md  # itch.io integration guide
│   │   ├── CREATING_PLUGINS.md    # How to create plugins
│   │   └── CONFIGURATION.md    # Configuration guide
│   └── CHANGELOG.md
│
├── system/                     # System integration files
│   ├── openconsole.service     # Systemd service file
│   ├── install.sh              # Installation script
│   ├── build-pi-image.sh       # Raspberry Pi image builder
│   └── configure-controllers.sh  # Controller setup utility
│
├── CMakeLists.txt              # Root CMake configuration
├── README.md                   # Main README
└── CLAUDE.md                   # This file

⭐ = OpenConsole-specific additions/major modifications
```

### Directory Purposes

| Directory | Purpose | When to Look Here |
|-----------|---------|-------------------|
| `es-core/src/components/` | Base UI components | Need to understand basic UI building blocks |
| `es-core/src/guis/` | Core dialogs and screens | Working with message boxes, popups |
| `es-app/src/guis/` | **OpenConsole screens** | **Adding settings, menus, dialogs** |
| `es-app/src/db/` | **Database operations** | **Game metadata, queries, schema** |
| `es-app/src/plugins/` | **Game sources** | **Adding new game sources, scanning** |
| `es-app/src/launchers/` | **Game execution** | **Supporting new game formats** |
| `es-app/src/api/` | **External APIs** | **itch.io, future integrations** |
| `es-app/src/utils/` | **OpenConsole utilities** | **Credential storage, helpers** |
| `es-app/src/views/` | View layer | Game list display, navigation |

---

## Key Components

### 1. GuiOpenConsoleSettings (`es-app/src/guis/GuiOpenConsoleSettings.cpp`)

**Purpose**: Main configuration menu for OpenConsole features

**Location in UI**: Main Menu → "OPENCONSOLE SETTINGS"

**Key Features**:
- Database management (view stats, clear, refresh)
- itch.io authentication and configuration (built-in, always visible)
- Plugin settings (scan paths, recursive scan depth)
- Game scanning trigger (fully functional)
- Maintenance operations

**Recent Changes**:
- Now accessible in all UI modes (was only in "Full" mode)
- itch.io section always visible (was conditionally shown)
- Game scanning fully implemented with detailed statistics
- Fixed crash when opening from start screen

**When to Modify**:
- Adding new OpenConsole settings
- Integrating new plugins or features
- Changing scanning behavior

### 2. DatabaseManager (`es-app/src/db/DatabaseManager.cpp`)

**Purpose**: SQLite database interface for game metadata

**Pattern**: Singleton

**Schema**:
- `games` table: Main game metadata (name, path, description, cover art, play stats)
- `itch_games` table: itch.io-specific data (itch ID, upload ID, download URL)

**Key Methods**:
- `init()`: Initialize database and create schema
- `insertGame()`: Add game to database
- `getAllGames()`: Retrieve all games
- `updatePlayStats()`: Update play count and last played
- `beginTransaction()`, `commitTransaction()`: Batch operations

**When to Modify**:
- Adding new game metadata fields
- Creating new tables for features
- Optimizing queries

### 3. PluginManager (`es-app/src/plugins/PluginManager.cpp`)

**Purpose**: Manage game source plugins

**Pattern**: Singleton

**Registered Plugins**:
- `LocalFilesystemPlugin`: Scans local directories (always active)
- `ItchIoPlugin`: Fetches itch.io library (requires authentication)

**Key Methods**:
- `registerPlugin()`: Register a new plugin
- `getAuthenticatedPlugins()`: Get plugins ready to scan
- `getPlugin()`: Get specific plugin by name

**When to Modify**:
- Adding new game source plugins (Steam, GOG, Epic)
- Changing plugin initialization order

### 4. GameScanner (`es-app/src/plugins/GameScanner.cpp`)

**Purpose**: Orchestrate game scanning from all sources

**Key Methods**:
- `scanAllSources()`: Scan all authenticated plugins, return statistics

**Statistics Returned**:
- Total games found
- New games added
- Games skipped (duplicates)
- Errors encountered
- Scan duration

**Recent Changes**:
- Now fully functional (was stub implementation)
- Provides detailed statistics for UI display
- Automatically triggers game list reload

**When to Modify**:
- Adding progress reporting
- Implementing parallel scanning
- Adding scan filters

### 5. ItchIoPlugin (`es-app/src/plugins/ItchIoPlugin.cpp`)

**Purpose**: itch.io game source integration

**Recent Changes**:
- Now treated as built-in feature (always visible in settings)
- Authentication always accessible regardless of UI mode

**Key Methods**:
- `authenticate()`: Test API key, fetch user profile
- `fetchGames()`: Get owned games from itch.io library
- `downloadGame()`: Download game with progress callback

**Dependencies**:
- `ItchIoApiClient`: HTTP client for itch.io API
- `TokenStorage`: Secure API key storage

**When to Modify**:
- Adding game update detection
- Implementing selective file downloads
- Adding cover art caching

### 6. TokenStorage (`es-app/src/utils/TokenStorage.cpp`)

**Purpose**: Secure credential storage with AES-256 encryption

**Pattern**: Singleton

**Security**:
- AES-256-CBC encryption
- Machine-specific keys (hostname + home path + salt)
- File permissions: 0600
- Storage location: `~/.openconsole/credentials.enc`

**Key Methods**:
- `storeToken()`: Encrypt and save credential
- `getToken()`: Decrypt and retrieve credential
- `hasToken()`: Check if credential exists
- `removeToken()`: Delete specific credential

**When to Modify**:
- Adding new credential types
- Implementing key rotation
- Adding backup/restore

### 7. OpenConsoleSystem (`es-app/src/OpenConsoleSystem.cpp`)

**Purpose**: Bridge between database and EmulationStation's UI

**Pattern**: Singleton (static methods)

**Key Methods**:
- `init()`: Initialize database and plugins
- `createSystem()`: Create "OpenConsole" system from database
- `refreshGameList()`: Reload games from database to UI
- `deinit()`: Cleanup

**Data Conversion**:
```
GameMetadata (Database) → FileData (UI)
```

**When to Modify**:
- Adding new metadata fields to UI
- Changing game list organization
- Implementing filters/sorting

### 8. GuiVirtualKeyboard (`es-core/src/guis/GuiVirtualKeyboard.cpp`)

**Purpose**: On-screen QWERTY keyboard for controller input

**Layout**: QWERTY with shift support

**Controller Mappings**:
- D-pad: Navigate keyboard
- A: Select character / Submit
- B: Backspace / Cancel
- X: Space
- Y: Toggle shift
- Start: Accept and submit

**When to Modify**:
- Adding alternate layouts (AZERTY, Dvorak)
- Adding special character mode
- Implementing text prediction

---

## Recent Fixes & Improvements

### Fix #1: Menu Crashes (Commit c6e0136)

**Problem**: Opening OpenConsole Settings from the start screen caused assertion failures

**Root Cause**: `getHelpStyle()` in `GuiSettings` and `GuiScreensaverOptions` called `getState().getSystem()` without checking if the viewing mode was valid

**Solution**: Added viewing mode checks before accessing system:
```cpp
// es-app/src/guis/GuiSettings.cpp
std::vector<HelpPrompt> GuiSettings::getHelpPrompts() {
    // Only try to get system if we're in a valid viewing mode
    if (ViewController::get()->getState().viewing == ViewController::GAME_LIST ||
        ViewController::get()->getState().viewing == ViewController::SYSTEM_SELECT) {
        // Safe to access system
    }
}
```

**Files Modified**:
- `es-app/src/guis/GuiSettings.cpp`
- `es-app/src/guis/GuiScreensaverOptions.cpp`

**Impact**: Improved stability, can now open settings from any screen

### Fix #2: Startup Black Screen (Commit c6e0136)

**Problem**: After dismissing "No games found yet" welcome dialog, screen was black

**Root Cause**: Welcome dialog closed, but no UI was shown afterward

**Solution**: Added menu callback to automatically open main menu after dialog dismissal:
```cpp
// es-app/src/views/ViewController.cpp
msgBox->setCallback([this] {
    // Open main menu after welcome dialog is dismissed
    mWindow->pushGui(new GuiMenu(mWindow));
});
```

**Files Modified**:
- `es-app/src/views/ViewController.cpp`

**Impact**: Better first-run experience, users can immediately access settings

### Fix #3: OPENCONSOLE SETTINGS Accessibility (Commit dc66219)

**Problem**: OPENCONSOLE SETTINGS menu only visible when UI mode = "Full"

**Root Cause**: Menu item wrapped in UI mode conditional check

**Solution**: Moved OPENCONSOLE SETTINGS outside UI mode restriction:
```cpp
// es-app/src/guis/GuiMenu.cpp
// OpenConsole Settings - always accessible
addEntry("OPENCONSOLE SETTINGS", 0x777777FF, true, [this] {
    mWindow->pushGui(new GuiOpenConsoleSettings(mWindow));
});
```

**Files Modified**:
- `es-app/src/guis/GuiMenu.cpp`

**Impact**: itch.io configuration and settings now accessible in Kid, Kiosk, and other UI modes

### Fix #4: itch.io Built-in Integration (Commit 0bbae2f)

**Problem**:
1. itch.io settings hidden behind plugin availability check
2. Game scanning was stub implementation

**Solution**:
1. Removed plugin conditional check - itch.io always shown as built-in feature
2. Implemented full game scanning with statistics

**Implementation**:
```cpp
// es-app/src/guis/GuiOpenConsoleSettings.cpp

// itch.io section always visible
addEntry("itch.io Status: " + status, statusColor, false, nullptr);

// Scanning implementation
void GuiOpenConsoleSettings::scanGames() {
    auto stats = GameScanner::scanAllSources([](int current, int total, const std::string& status) {
        // Progress callback
    });

    // Show detailed results
    std::stringstream ss;
    ss << "Scan Complete!\n\n"
       << "Games found: " << stats.totalGamesFound << "\n"
       << "New games added: " << stats.newGamesAdded << "\n"
       << "Games skipped: " << stats.gamesSkipped << "\n"
       << "Errors: " << stats.errors << "\n"
       << "Duration: " << stats.scanDurationSeconds << "s";

    // Reload game list
    OpenConsoleSystem::refreshGameList();
}
```

**Files Modified**:
- `es-app/src/guis/GuiOpenConsoleSettings.cpp`

**Impact**: itch.io always accessible, scanning fully functional with detailed feedback

---

## Common Development Tasks

### Adding a New Game Source Plugin

**Example**: Adding Steam integration

1. **Create plugin interface implementation**:
   ```cpp
   // es-app/src/plugins/SteamPlugin.h
   class SteamPlugin : public IGameSourcePlugin {
   public:
       std::string getName() const override { return "steam"; }
       bool initialize() override;
       std::vector<GameMetadata> fetchGames() override;
   };
   ```

2. **Register plugin**:
   ```cpp
   // es-app/src/OpenConsoleSystem.cpp
   PluginManager::getInstance().registerPlugin(
       std::make_shared<SteamPlugin>()
   );
   ```

3. **Add to CMakeLists.txt**:
   ```cmake
   es-app/src/plugins/SteamPlugin.cpp
   es-app/src/plugins/SteamPlugin.h
   ```

4. **Add settings UI**:
   ```cpp
   // es-app/src/guis/GuiOpenConsoleSettings.cpp
   addEntry("Configure Steam", 0x777777FF, true, [this] {
       // Open Steam auth dialog
   });
   ```

### Adding a New Game Launcher

**Example**: Adding support for Unity games

1. **Create launcher implementation**:
   ```cpp
   // es-app/src/launchers/UnityLauncher.h
   class UnityLauncher : public IGameLauncher {
   public:
       bool canLaunch(const GameMetadata& game) const override;
       bool launch(const GameMetadata& game) override;
   };
   ```

2. **Register in factory**:
   ```cpp
   // es-app/src/launchers/GameLauncherFactory.cpp
   if (game.gameType == GameType::UNITY) {
       return std::make_shared<UnityLauncher>();
   }
   ```

3. **Add game type detection**:
   ```cpp
   // es-app/src/launchers/GameLauncherFactory.cpp
   if (executablePath.find("Unity") != std::string::npos) {
       return GameType::UNITY;
   }
   ```

### Adding a New Database Table

**Example**: Adding achievements table

1. **Update schema**:
   ```cpp
   // es-app/src/db/DatabaseManager.cpp - init()
   const char* createAchievements = R"(
       CREATE TABLE IF NOT EXISTS achievements (
           id INTEGER PRIMARY KEY AUTOINCREMENT,
           game_id INTEGER NOT NULL,
           name TEXT NOT NULL,
           unlocked INTEGER DEFAULT 0,
           FOREIGN KEY(game_id) REFERENCES games(id)
       );
   )";
   sqlite3_exec(db, createAchievements, nullptr, nullptr, nullptr);
   ```

2. **Add methods**:
   ```cpp
   // es-app/src/db/DatabaseManager.h
   bool addAchievement(int gameId, const std::string& name);
   std::vector<Achievement> getAchievements(int gameId);
   ```

3. **Update metadata struct** (if needed):
   ```cpp
   // es-app/src/db/DatabaseManager.h
   struct GameMetadata {
       // existing fields...
       std::vector<Achievement> achievements;
   };
   ```

### Adding a Settings Menu Item

**Example**: Adding a theme selector

1. **Add to GuiOpenConsoleSettings**:
   ```cpp
   // es-app/src/guis/GuiOpenConsoleSettings.cpp

   // In constructor
   ComponentListRow themeRow;
   auto theme = std::make_shared<TextComponent>(
       mWindow, "Theme", Font::get(FONT_SIZE_MEDIUM), 0x777777FF
   );
   themeRow.addElement(theme, true);
   themeRow.makeAcceptInputHandler([this] {
       openThemeSelector();
   });
   addRow(themeRow);
   ```

2. **Implement handler**:
   ```cpp
   void GuiOpenConsoleSettings::openThemeSelector() {
       // Open theme selection dialog
       auto themeList = getAvailableThemes();
       mWindow->pushGui(new GuiThemeSelector(mWindow, themeList));
   }
   ```

---

## Architecture Quick Reference

### Data Flow Diagrams

#### Game Scan Flow
```
User clicks "Scan for Games Now"
    ↓
GuiOpenConsoleSettings::scanGames()
    ↓
GameScanner::scanAllSources()
    ↓
PluginManager::getAuthenticatedPlugins()
    ↓
For each plugin:
    plugin->fetchGames()
    ↓
    LocalFilesystemPlugin: Scan directories
    ItchIoPlugin: Fetch from itch.io API
    ↓
    Return GameMetadata[]
    ↓
DatabaseManager::insertGame() for each game
    ↓
Progress callback updates UI
    ↓
OpenConsoleSystem::refreshGameList()
    ↓
UI shows updated game list with statistics
```

#### Game Launch Flow
```
User selects game in UI
    ↓
FileData::launchGame()
    ↓
Get game ID from metadata
    ↓
DatabaseManager::getGameById()
    ↓
GameLauncherFactory::createLauncher(game)
    ↓
Detect game type, create appropriate launcher:
    - AppImageLauncher
    - RenpyLauncher
    - DebLauncher
    ↓
launcher->launch(game)
    ↓
Fork process, exec game
    ↓
Wait for game to exit
    ↓
DatabaseManager::updatePlayStats()
    ↓
Return to UI
```

#### itch.io Authentication Flow
```
User selects "Configure itch.io"
    ↓
GuiOpenConsoleSettings → GuiItchIoAuth
    ↓
User selects "ENTER API KEY"
    ↓
GuiVirtualKeyboard opens
    ↓
User enters API key with controller
    ↓
ItchIoPlugin::setApiKey(key)
    ↓
ItchIoPlugin::authenticate()
    ↓
ItchIoApiClient::testApiKey()
    ↓
HTTP GET /profile
    ↓
Parse JSON, extract username
    ↓
TokenStorage::storeToken("itch_io", key)
    ↓
Encrypt with AES-256, save to credentials.enc
    ↓
Display "Authenticated as: username"
    ↓
Status updates to "Authenticated" (green)
```

### Design Patterns Used

| Pattern | Where Used | Purpose |
|---------|------------|---------|
| **Singleton** | DatabaseManager, PluginManager, TokenStorage, OpenConsoleSystem | Single instance management |
| **Factory** | GameLauncherFactory | Create launchers based on game type |
| **Strategy** | IGameSourcePlugin, IGameLauncher | Interchangeable algorithms |
| **Observer** | GUI events, input handling | Event propagation |
| **Component** | GuiComponent hierarchy | UI composition |
| **Bridge** | OpenConsoleSystem | Database ↔ UI abstraction |

### Threading Model

OpenConsole is **primarily single-threaded**:
- Main thread handles: SDL events → input → update → render → swap buffers
- Async operations: texture loading, HTTP requests (with callbacks)
- **No worker threads for game scanning** (runs on main thread with callbacks for UI updates)

---

## Testing & Debugging

### Build and Run

```bash
# Clean build
rm -rf build && mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Run
./emulationstation

# Run with verbose logging
./emulationstation --debug
```

### Log Files

**Main Log**: `~/.emulationstation/es_log.txt`

Log levels:
- `LogError`: Critical errors
- `LogWarning`: Warnings
- `LogInfo`: General information
- `LogDebug`: Verbose debugging (enabled with --debug flag)

**Adding Logs**:
```cpp
#include "Log.h"

LOG(LogInfo) << "Starting game scan...";
LOG(LogError) << "Failed to open database: " << errorMsg;
LOG(LogDebug) << "Found " << games.size() << " games";
```

### Database Inspection

```bash
# Open database
sqlite3 ~/.openconsole/games.db

# View schema
.schema

# List games
SELECT id, name, game_type, source FROM games;

# View itch.io games
SELECT * FROM itch_games;

# Check play stats
SELECT name, play_count, last_played FROM games ORDER BY play_count DESC LIMIT 10;
```

### Common Issues

#### Issue: Menu crashes on open
**Solution**: Check if viewing mode is valid before accessing `getState().getSystem()`
```cpp
if (ViewController::get()->getState().viewing == ViewController::GAME_LIST) {
    // Safe to access system
}
```

#### Issue: Settings not saving
**Solution**: Call `Settings::getInstance()->saveFile()` after changes

#### Issue: Games not appearing after scan
**Checklist**:
1. Check `es_log.txt` for errors
2. Verify database has entries: `sqlite3 ~/.openconsole/games.db "SELECT COUNT(*) FROM games;"`
3. Call `OpenConsoleSystem::refreshGameList()` after scanning
4. Ensure `OpenConsoleSystem::createSystem()` returned non-null

#### Issue: itch.io authentication fails
**Checklist**:
1. Check API key is correct (no extra spaces)
2. Verify network connectivity
3. Check `es_log.txt` for HTTP errors
4. Test API key manually: `curl -H "Authorization: Bearer YOUR_KEY" https://itch.io/api/1/profile`

### Debugging Tips

1. **UI Issues**: Check `Window` GUI stack: `mWindow->peekGui()`
2. **Database Issues**: Enable SQLite logging in `DatabaseManager::init()`
3. **Plugin Issues**: Check `PluginManager::getAuthenticatedPlugins()` returns expected plugins
4. **Rendering Issues**: Check OpenGL errors, verify theme XML is valid

---

## Important Files

### Configuration Files

| File | Purpose | Format |
|------|---------|--------|
| `~/.emulationstation/es_settings.cfg` | User settings | Key=Value |
| `~/.emulationstation/es_input.cfg` | Controller mappings | XML |
| `~/.emulationstation/es_systems.cfg` | System definitions | XML |
| `~/.openconsole/games.db` | Game database | SQLite3 |
| `~/.openconsole/credentials.enc` | Encrypted credentials | Binary (AES-256) |
| `~/.openconsole/config.xml` | OpenConsole settings | XML |

### Build Files

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Root build configuration |
| `es-core/CMakeLists.txt` | Core library build |
| `es-app/CMakeLists.txt` | Application build |
| `CMake/*.cmake` | CMake modules for dependencies |

### Entry Points

| File | Purpose |
|------|---------|
| `es-app/src/main.cpp` | Application entry point |
| `es-app/src/OpenConsoleSystem.cpp::init()` | OpenConsole initialization |
| `es-core/src/Window.cpp` | GUI system initialization |

---

## Development Workflow

### Typical Feature Development

1. **Understand the requirement**
   - Read existing documentation
   - Check similar features in codebase
   - Identify affected components

2. **Plan the implementation**
   - UI changes: Which `Gui*` classes?
   - Data changes: Database schema updates?
   - Logic changes: New plugins, launchers?
   - Settings: Update `GuiOpenConsoleSettings`?

3. **Make the changes**
   - Follow existing code style
   - Use smart pointers (`std::shared_ptr`, `std::unique_ptr`)
   - Add logging for important operations
   - Handle errors gracefully

4. **Test the changes**
   - Build and run locally
   - Check log file for errors
   - Test with controller (not just keyboard)
   - Verify settings persist across restarts

5. **Update documentation**
   - Update relevant docs in `docs/`
   - Add comments for complex logic
   - Update CHANGELOG.md

6. **Commit and push**
   - Write clear commit message
   - Reference issues if applicable
   - Push to feature branch

### Code Style Guidelines

- **Indentation**: Tabs (inherited from EmulationStation)
- **Braces**: Opening brace on same line
- **Naming**:
  - Classes: `PascalCase`
  - Methods: `camelCase`
  - Private members: `mMemberName`
  - Constants: `UPPER_SNAKE_CASE`
- **Includes**: Group by: C++ standard library, third-party, project headers
- **Comments**: Use `//` for single-line, `/* */` for multi-line

### Git Workflow

```bash
# Start new feature
git checkout -b feature/my-feature

# Make changes
git add .
git commit -m "Add feature description"

# Push to branch
git push -u origin feature/my-feature

# Create PR when ready
```

---

## Quick Navigation Guide

**Want to...**

- **Add a new settings menu item?** → `es-app/src/guis/GuiOpenConsoleSettings.cpp`
- **Add support for a new game format?** → `es-app/src/launchers/` (create new launcher)
- **Add a new game source (Steam, GOG)?** → `es-app/src/plugins/` (create new plugin)
- **Change database schema?** → `es-app/src/db/DatabaseManager.cpp::init()`
- **Add new UI component?** → `es-core/src/guis/` or `es-app/src/guis/`
- **Fix controller input?** → `es-core/src/InputManager.cpp`
- **Change game scanning logic?** → `es-app/src/plugins/GameScanner.cpp`
- **Modify itch.io integration?** → `es-app/src/plugins/ItchIoPlugin.cpp`, `es-app/src/api/ItchIoApiClient.cpp`
- **Update theme system?** → `es-core/src/ThemeData.cpp`
- **Change how games are displayed?** → `es-app/src/views/gamelist/`
- **Add encrypted storage?** → `es-app/src/utils/TokenStorage.cpp`

---

## Additional Resources

- **Main Documentation**: `docs/ARCHITECTURE.md`
- **Plugin Development**: `docs/architecture/PLUGINS.md`
- **Database Details**: `docs/architecture/DATABASE.md`
- **UI Components**: `docs/guides/UI_COMPONENTS.md`
- **itch.io Integration**: `docs/guides/ITCHIO_INTEGRATION.md`
- **Original EmulationStation**: https://github.com/RetroPie/EmulationStation

---

## Version History

**Last Updated**: 2025-11-23

**Recent Changes**:
- Fixed menu crashes when opening from start screen
- Fixed startup black screen issue
- Made OPENCONSOLE SETTINGS accessible in all UI modes
- Made itch.io integration built-in (always visible)
- Implemented full game scanning with detailed statistics

**Contributors**: Claude (AI Assistant)

---

**Happy Coding! 🎮**
