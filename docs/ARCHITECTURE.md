# OpenConsole Architecture

This document provides a comprehensive overview of the OpenConsole architecture, including both the inherited EmulationStation components and our new OpenConsole-specific features.

## Table of Contents

1. [High-Level Architecture](#high-level-architecture)
2. [Layer Architecture](#layer-architecture)
3. [Core Components](#core-components)
4. [Data Flow](#data-flow)
5. [Design Patterns](#design-patterns)
6. [OpenConsole Extensions](#openconsole-extensions)

## High-Level Architecture

OpenConsole follows a layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│                    User Interface Layer                  │
│  (Views, GUI Components, Theme System, Input Handling)   │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                      │
│     (Game Management, Plugin System, Launcher System)    │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                     Data Layer                           │
│        (Database Manager, File System, Settings)         │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│                   Platform Layer                         │
│  (SDL2, OpenGL, Operating System, Hardware Interfaces)   │
└─────────────────────────────────────────────────────────┘
```

## Layer Architecture

### 1. Platform Layer
**Location**: `es-core/src/platform.cpp`, SDL2, OpenGL

**Responsibilities**:
- Window management and rendering
- Input device handling (keyboard, gamepad, CEC)
- Audio output
- File system operations
- Process execution

**Key Technologies**:
- SDL2 for windowing and input
- OpenGL/OpenGL ES for rendering
- ALSA for audio (Linux)
- POSIX APIs for process management

### 2. Core Rendering Layer
**Location**: `es-core/src/`

**Responsibilities**:
- Component-based UI framework
- Theme engine
- Resource management (fonts, textures)
- Animation system
- Input routing

**Key Classes**:
- `Window` - Main window and GUI stack manager
- `GuiComponent` - Base class for all UI elements
- `Renderer` - Abstract rendering interface
- `ThemeData` - Theme parsing and application
- `InputManager` - Input device management

### 3. Application Layer
**Location**: `es-app/src/`

**Responsibilities**:
- Game discovery and management
- System/collection organization
- Metadata scraping
- Game launching
- **[NEW]** Plugin system
- **[NEW]** Game scanner
- **[NEW]** Database management

**Key Classes**:
- `SystemData` - Represents a game system/collection
- `FileData` - Represents a game or folder
- `ViewController` - Main application controller
- **[NEW]** `PluginManager` - Plugin lifecycle management
- **[NEW]** `GameScanner` - Game scanning orchestration
- **[NEW]** `DatabaseManager` - SQLite database operations

### 4. Data Layer
**Location**: `es-app/src/db/`, configuration files

**Responsibilities**:
- Persistent storage (database, XML)
- Configuration management
- Game metadata storage
- Plugin data management

**Key Components**:
- **[NEW]** SQLite3 database (games, itch.io data)
- XML configuration files (settings, systems)
- Theme files (XML)
- Gamelist files (XML - legacy)

## Core Components

### Component-Based UI System

All UI elements inherit from `GuiComponent`:

```cpp
GuiComponent (virtual base)
├── TextComponent
├── ImageComponent
├── ButtonComponent
├── MenuComponent
├── ComponentList
├── TextListComponent<T>
└── [Custom Components...]
```

**Key Features**:
- Position, size, and transform (rotation, scale)
- Z-ordering and visibility
- Input handling (with propagation)
- Animation support
- Theme property application

### View System

The view system manages different UI screens:

```cpp
ViewController (Singleton)
├── SystemView (carousel of systems)
└── IGameListView (per-system views)
    ├── BasicGameListView (text list)
    ├── DetailedGameListView (list + metadata)
    ├── GridGameListView (grid of images)
    └── VideoGameListView (with video preview)
```

### Theme System

Themes are XML-based and support:
- Element positioning and sizing
- Colors, fonts, images
- Animations and transitions
- Multiple view types
- Resolution-independent design

Example theme snippet:
```xml
<theme>
  <formatVersion>3</formatVersion>
  <view name="system">
    <text name="title">
      <pos>0.5 0.1</pos>
      <color>FFFFFF</color>
    </text>
  </view>
</theme>
```

## OpenConsole Extensions

### UI Components

```
GuiVirtualKeyboard (GuiComponent)
└── On-screen keyboard with controller navigation

GuiOpenConsoleSettings (GuiSettings)
└── OpenConsole configuration menu

OpenConsoleSystem (singleton)
└── Database-to-FileData bridge
```

**Design**: Component pattern with controller-first UX
**Purpose**: User interface for OpenConsole features

Key UI Components:
- **GuiVirtualKeyboard**: QWERTY keyboard for text input without physical keyboard
- **GuiOpenConsoleSettings**: Central hub for database, plugin, and scanning configuration
- **OpenConsoleSystem**: Integrates database games into EmulationStation's view system

### Plugin Architecture

```
IGameSourcePlugin (interface)
├── LocalFilesystemPlugin (local game scanning)
├── ItchIoPlugin (itch.io integration - built-in)
└── [Future plugins: Steam, GOG, Epic Games...]

PluginManager (singleton)
└── Manages plugin lifecycle
```

**Design**: Strategy pattern with interface-based plugins
**Purpose**: Extensible game source discovery

Built-in Plugins:
- **LocalFilesystemPlugin**: Scans local directories for games (always active)
- **ItchIoPlugin**: Fetches and downloads games from itch.io library (always available, requires authentication)

**Note**: As of recent updates, itch.io integration is built-in and always accessible in the OpenConsole Settings menu, regardless of UI mode.

See [Plugin System Documentation](architecture/PLUGINS.md) and [itch.io Integration Guide](guides/ITCHIO_INTEGRATION.md)

### Launcher System

```
IGameLauncher (interface)
├── AppImageLauncher
├── RenpyLauncher
├── DebLauncher
└── [Future launchers...]

GameLauncherFactory
└── Factory pattern for launcher creation
```

**Design**: Factory pattern with type detection
**Purpose**: Execute different game formats

See [Launcher System Documentation](architecture/LAUNCHERS.md)

### Database Layer

```
DatabaseManager (singleton)
└── SQLite3 database
    ├── games table
    └── itch_games table
```

**Design**: Singleton with prepared statements
**Purpose**: Fast, reliable game metadata storage

See [Database Documentation](architecture/DATABASE.md)

### Game Scanner

```
GameScanner
├── Uses PluginManager to fetch games
├── Uses DatabaseManager to store metadata
└── Provides progress callbacks for UI
```

**Design**: Orchestrator pattern
**Purpose**: Coordinate scanning and database population

### Token Storage

```
TokenStorage (singleton)
└── AES-256-CBC encrypted credential storage
    ├── Machine-specific encryption key
    ├── SHA-256 key derivation
    └── JSON format storage
```

**Design**: Singleton with encryption
**Purpose**: Secure credential and API token storage

Features:
- AES-256-CBC encryption for all stored tokens
- Machine-specific keys (hostname + home path + salt)
- File permissions: 0600 (owner read/write only)
- Storage location: `~/.openconsole/credentials.enc`

### itch.io API Client

```
ItchIoApiClient
├── HTTP client (libCURL)
├── JSON parsing (RapidJSON)
└── Progress callbacks
```

**Design**: HTTP client wrapper
**Purpose**: Communication with itch.io API

Endpoints:
- **GET /profile**: User profile information
- **GET /profile/owned-keys**: Owned games library
- **GET /uploads/{id}/download**: Download URLs

## Data Flow

### Game Launch Flow

```
User Input (Controller)
    ↓
InputManager → Window → ViewController
    ↓
IGameListView → FileData::launchGame()
    ↓
GameLauncherFactory::createLauncher()
    ↓
IGameLauncher::launch()
    ↓
fork() + exec() → Game Process
    ↓
waitpid() → Return to UI
    ↓
DatabaseManager::updatePlayStats()
```

### Game Scanning Flow

```
User Action (Scan Games)
    ↓
GuiOpenConsoleSettings::scanGames()
    ↓
GameScanner::scanAllSources()
    ↓
PluginManager::getAuthenticatedPlugins()
    ↓
LocalFilesystemPlugin::fetchGames()
    ↓
GameLauncherFactory::detectGameType()
    ↓
Extract metadata + Find cover art
    ↓
Return GameMetadata[]
    ↓
DatabaseManager::insertGame()
    ↓
Progress callback to UI
    ↓
OpenConsoleSystem::refreshGameList()
    ↓
UI Updates with new games
```

### Virtual Keyboard Flow

```
User Action (Add Scan Path)
    ↓
GuiOpenConsoleSettings::addScanPath()
    ↓
GuiVirtualKeyboard created with callback
    ↓
User navigates with D-pad (←↑→↓)
    ↓
User selects characters (A button)
    ↓
User completes text (Start button)
    ↓
Callback invoked with text string
    ↓
GuiOpenConsoleSettings receives path
    ↓
Path added to configuration
    ↓
Virtual keyboard closes
```

### Database Integration Flow

```
Application Startup
    ↓
OpenConsoleSystem::init()
    ↓
DatabaseManager::init()
    ↓
PluginManager::init()
    ↓
OpenConsoleSystem::createSystem()
    ↓
DatabaseManager::getAllGames()
    ↓
Convert GameMetadata → FileData
    ↓
Create SystemData for "OpenConsole"
    ↓
Add to ViewController
    ↓
Games appear in UI
```

### itch.io Authentication Flow

```
User Action (Configure itch.io)
    ↓
GuiOpenConsoleSettings::openItchIoAuth()
    ↓
GuiItchIoAuth created
    ↓
User selects "VIEW INSTRUCTIONS"
    ↓
Display API key instructions
    ↓
User visits itch.io/user/settings/api-keys
    ↓
User generates API key on website
    ↓
User selects "ENTER API KEY"
    ↓
GuiVirtualKeyboard opens
    ↓
User enters API key with controller
    ↓
ItchIoPlugin::setApiKey()
    ↓
ItchIoPlugin::authenticate()
    ↓
ItchIoApiClient::testApiKey()
    ↓
HTTP GET /profile (with API key)
    ↓
Parse JSON response
    ↓
Extract username and user ID
    ↓
TokenStorage::storeToken("itch_io", apiKey)
    ↓
Encrypt with AES-256-CBC
    ↓
Save to ~/.openconsole/credentials.enc
    ↓
AuthResult returned (success + username)
    ↓
Display "Authenticated as: {username}"
    ↓
itch.io games available in scans
```

### itch.io Game Download Flow

```
User Action (Scan for Games)
    ↓
GameScanner::scanAllSources()
    ↓
PluginManager::getAuthenticatedPlugins()
    ↓
ItchIoPlugin::isAuthenticated() → true
    ↓
ItchIoPlugin::fetchGames()
    ↓
TokenStorage::getToken("itch_io")
    ↓
Decrypt stored API key
    ↓
ItchIoApiClient::getOwnedGames()
    ↓
HTTP GET /profile/owned-keys?api_key={key}
    ↓
Parse owned games JSON
    ↓
Convert ItchIoGame → GameMetadata
    ↓
Detect game type from filename
    ↓
Return games to scanner
    ↓
DatabaseManager::insertGame()
    ↓
Games appear in UI
    ↓
[User selects game to download]
    ↓
ItchIoPlugin::downloadGame()
    ↓
ItchIoApiClient::getDownloadUrl(uploadId)
    ↓
HTTP GET /uploads/{id}/download
    ↓
Get temporary download URL
    ↓
ItchIoApiClient::downloadFile(url, path, progressCallback)
    ↓
CURL download with progress tracking
    ↓
Progress callback updates UI
    ↓
File saved to ~/.openconsole/downloads/itch.io/{id}/
    ↓
Download complete
```

## Design Patterns

### Singleton Pattern
Used for manager classes that should have single instances:
- `PluginManager`
- `DatabaseManager`
- `TokenStorage`
- `OpenConsoleSystem`
- `ViewController`
- `InputManager`
- `Settings`

### Factory Pattern
Used for creating appropriate instances based on type:
- `GameLauncherFactory` - Creates launchers by game type
- Renderer creation - Creates appropriate OpenGL renderer

### Strategy Pattern
Used for interchangeable algorithms:
- `IGameSourcePlugin` - Different game source strategies
- `IGameLauncher` - Different launch strategies
- `IGameListView` - Different view strategies

### Observer Pattern
Used for event notification:
- Input events propagate through GUI stack
- Animation callbacks
- Progress callbacks in scanning

### Component Pattern
Used for UI system:
- All UI elements are `GuiComponent` instances
- Composition over inheritance
- Transform hierarchy

## Threading Model

OpenConsole is primarily single-threaded with async operations for:
- Texture loading (`TextureDataManager`)
- HTTP requests (`AsyncReqComponent` for scrapers)
- Game scanning (can use callbacks for progress)

**Main Thread**:
1. Event polling (SDL)
2. Input processing
3. Update (animations, logic)
4. Render
5. SwapBuffers

## Memory Management

- **Smart Pointers**: Used extensively (`std::shared_ptr`, `std::unique_ptr`)
- **RAII**: Resources tied to object lifetime
- **Manual Management**: Some legacy EmulationStation code uses raw pointers
- **Singletons**: Self-managed lifecycle

## Error Handling

- **Logging**: Comprehensive logging via `Log` system
  - `LogError`, `LogWarning`, `LogInfo`, `LogDebug`
  - Logs to file: `~/.emulationstation/es_log.txt`
- **Exceptions**: Used sparingly, mainly in file operations
- **Return Values**: Boolean success/failure common
- **Error Strings**: `getLastError()` pattern for detailed errors

## Configuration

### Configuration Files

```
~/.emulationstation/
├── es_settings.cfg          # User settings (key=value)
├── es_systems.cfg           # System definitions (XML)
├── es_input.cfg            # Controller mappings (XML)
├── es_log.txt              # Application log
└── ~/.openconsole/
    ├── games.db             # SQLite game database
    ├── credentials.enc      # Encrypted API tokens
    └── config.xml          # OpenConsole config
```

## Performance Considerations

### Optimization Strategies

1. **Prepared SQL Statements**: Reuse for repeated queries
2. **Texture Caching**: Loaded textures cached in memory
3. **Lazy Loading**: Resources loaded on-demand
4. **Async Loading**: Non-blocking texture loading
5. **Index Optimization**: Database indices on frequently queried columns

### Target Performance

- **Raspberry Pi 5**: Primary target
- **60 FPS UI**: Maintained in menus and carousels
- **Fast Scanning**: <5 seconds for 100 games
- **Low Memory**: <512MB typical usage

## Extension Points

### Adding New Features

1. **New Game Source**: Implement `IGameSourcePlugin`
2. **New Game Format**: Implement `IGameLauncher`
3. **New UI View**: Extend `IGameListView`
4. **New GUI Component**: Extend `GuiComponent`
5. **New Settings Menu**: Extend `GuiSettings` (see `GuiOpenConsoleSettings`)
6. **New Input Component**: Use `GuiVirtualKeyboard` as reference
7. **New Theme Elements**: Extend `ThemeData`

## Dependencies

### Required
- SDL2 (>= 2.0.0)
- FreeImage
- FreeType
- libVLC (>= 3.0)
- libCURL
- SQLite3 (>= 3.0)
- RapidJSON
- OpenSSL (libcrypto) - for AES-256 encryption

### Optional
- libCEC (for TV control)
- ALSA (Linux audio)

## Build Targets

- **Desktop GL**: OpenGL 2.1 or 1.4
- **Embedded GLES**: OpenGL ES 2.0 or 1.0
- **Raspberry Pi**: Optimized builds with hardware acceleration

## Future Architecture

### Planned Improvements

1. **Async Scanning**: Non-blocking game scanning
2. **Plugin Hot-Reload**: Load plugins without restart
3. **Database Migrations**: Versioned schema updates
4. **Cloud Sync**: Optional save game sync
5. **Multi-User**: User profiles and game sharing

## Related Documentation

- [Database Layer Details](architecture/DATABASE.md)
- [Plugin System Details](architecture/PLUGINS.md)
- [Launcher System Details](architecture/LAUNCHERS.md)
- [UI Components Guide](guides/UI_COMPONENTS.md)
- [itch.io Integration Guide](guides/ITCHIO_INTEGRATION.md)
- [Development Guide](development/GETTING_STARTED.md)
