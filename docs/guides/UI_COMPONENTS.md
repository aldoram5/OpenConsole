# OpenConsole UI Components Guide

This guide covers the OpenConsole-specific UI components and how to use them in your development.

## Table of Contents

1. [Overview](#overview)
2. [GuiVirtualKeyboard](#guivirtualkeyboard)
3. [GuiOpenConsoleSettings](#guiopconsolesettings)
4. [OpenConsoleSystem](#openconsolesystem)
5. [Integration Examples](#integration-examples)
6. [Best Practices](#best-practices)

## Overview

OpenConsole extends EmulationStation's UI framework with controller-first components designed for living room gaming. All UI components follow these principles:

- **Controller-First**: No mouse/keyboard required
- **Consistent UX**: Follows EmulationStation's input conventions
- **Theme Compatible**: Respects theme system colors and fonts
- **Callback-Driven**: Asynchronous operations use callbacks

### EmulationStation UI Foundation

OpenConsole components build on EmulationStation's core UI classes:

- `GuiComponent` - Base class for all UI elements
- `GuiSettings` - Base class for settings menus
- `Window` - Main window and GUI stack manager
- `ComponentGrid` - Grid layout manager
- `MenuComponent` - Menu list component

## GuiVirtualKeyboard

On-screen keyboard for text input without a physical keyboard.

### Features

- **QWERTY Layout**: Standard keyboard layout with numbers row
- **Shift Support**: Toggle between lowercase and uppercase/symbols
- **Controller Navigation**: Full D-pad navigation
- **Callback System**: Invoke callback when text is accepted
- **Multi-line Support**: Optional multi-line text input

### Usage

```cpp
#include "guis/GuiVirtualKeyboard.h"

// Create virtual keyboard with callback
auto keyboard = std::make_shared<GuiVirtualKeyboard>(
    mWindow,
    "ENTER PATH",                    // Title
    "/home/user/Games",               // Initial text
    [this](const std::string& text) { // Callback
        // Handle the entered text
        Log::write(LogInfo, "User entered: " + text);
    },
    false,                            // Multi-line (false = single line)
    "OK"                              // Accept button text
);

// Push to GUI stack
mWindow->pushGui(keyboard);
```

### Controller Mappings

| Button | Action |
|--------|--------|
| D-pad ↑↓←→ | Navigate keyboard |
| A (Confirm) | Select character / Submit |
| B (Back) | Backspace / Cancel |
| X | Insert space |
| Y | Toggle shift |
| Start | Accept and submit text |

### Keyboard Layout

**Normal Mode:**
```
1 2 3 4 5 6 7 8 9 0
q w e r t y u i o p
a s d f g h j k l
z x c v b n m
```

**Shift Mode:**
```
! @ # $ % ^ & * ( )
Q W E R T Y U I O P
A S D F G H J K L
Z X C V B N M
```

### Implementation Details

- **Class**: `GuiVirtualKeyboard`
- **Inherits**: `GuiComponent`
- **Location**: `es-core/src/guis/GuiVirtualKeyboard.h/cpp`
- **Layout**: Uses `ComponentGrid` for structured keyboard layout
- **Text Display**: Shows current text being edited above keyboard
- **Auto-shift**: Shift automatically disables after one character

### Customization

```cpp
// Create keyboard with custom title and button text
auto keyboard = std::make_shared<GuiVirtualKeyboard>(
    mWindow,
    "CUSTOM TITLE",
    "",
    callback,
    true,        // Multi-line enabled
    "SUBMIT"     // Custom button text
);
```

## GuiOpenConsoleSettings

Comprehensive settings menu for OpenConsole configuration.

### Features

- **Database Management**: View path, statistics, maintenance
- **Plugin Configuration**: Manage scan paths and depth
- **Scan Control**: Manual and automatic scanning options
- **Progress Feedback**: Real-time updates during operations

### Usage

```cpp
#include "guis/GuiOpenConsoleSettings.h"

// Open OpenConsole settings
mWindow->pushGui(new GuiOpenConsoleSettings(mWindow));
```

### Menu Sections

#### 1. Database Section

**Database Path** - Displays current database file location
```
Database: ~/.openconsole/games.db
```

**View Statistics** - Opens dialog showing game counts
```cpp
void viewDatabaseStats();
```
Shows:
- Total games
- Games by type (AppImage, Ren'Py, .deb, etc.)
- Games by source (Local, itch.io, USB)

#### 2. Plugin Section

**Installed Plugins** - Shows count of registered plugins
```
Plugins: 1 installed
```

**Recursive Scan** - Toggle deep directory scanning
```cpp
auto recursive_scan = std::make_shared<SwitchComponent>(mWindow);
recursive_scan->setState(true);  // Enable recursive scanning
```

**Scan Depth** - Configure maximum directory depth
```cpp
void configureScanDepth();
```
Opens slider to set depth (1-10, default: 5)

**Manage Scan Paths** - Add/remove scan directories
```cpp
void addScanPath();
```
Opens `GuiVirtualKeyboard` to enter new path

#### 3. Scanning Section

**Scan for Games Now** - Manual scan trigger
```cpp
void scanGames();
```
Executes:
1. Creates progress dialog
2. Calls `GameScanner::scanAllSources()`
3. Updates UI with results
4. Refreshes game list

**Auto-scan on Startup** - Toggle automatic scanning
```cpp
auto auto_scan = std::make_shared<SwitchComponent>(mWindow);
Settings::getInstance()->setBool("AutoScanOnStartup", true);
```

#### 4. Maintenance Section

**Refresh Database** - Rescan all sources
```cpp
void refreshDatabase();
```
Clears and repopulates database from scratch

**Clear Database** - Delete all games (with confirmation)
```cpp
void clearDatabase();
```
Shows confirmation dialog before clearing

### Implementation Details

- **Class**: `GuiOpenConsoleSettings`
- **Inherits**: `GuiSettings`
- **Location**: `es-app/src/guis/GuiOpenConsoleSettings.h/cpp`
- **Uses**: `MenuComponent`, `SwitchComponent`, `SliderComponent`
- **Integration**: `GameScanner`, `DatabaseManager`, `OpenConsoleSystem`

### Progress Callbacks

Scanning operations use callbacks for UI updates:

```cpp
auto stats = GameScanner::scanAllSources([this](int current, int total, const std::string& status) {
    // Update progress dialog
    updateProgress(current, total, status);
});
```

## OpenConsoleSystem

Bridge between the database layer and EmulationStation's game list views.

### Features

- **Database Integration**: Converts `GameMetadata` to `FileData`
- **System Creation**: Creates virtual "OpenConsole" system
- **Game List Management**: Refresh and update game lists
- **Initialization**: Handles startup and cleanup

### Usage

```cpp
#include "OpenConsoleSystem.h"

// Initialize OpenConsole system
if (!OpenConsoleSystem::init()) {
    Log::write(LogError, "Failed to initialize OpenConsole system");
}

// Create the OpenConsole system
SystemData* ocSystem = OpenConsoleSystem::createSystem();
if (ocSystem) {
    SystemData::sSystemVector.push_back(ocSystem);
}

// Later: refresh game list after scanning
OpenConsoleSystem::refreshGameList();

// On shutdown
OpenConsoleSystem::deinit();
```

### API Reference

#### Static Methods

**init()**
```cpp
static bool init();
```
Initializes database and plugin manager. Call once at application startup.

**deinit()**
```cpp
static void deinit();
```
Cleanup and shutdown. Call once at application exit.

**createSystem()**
```cpp
static SystemData* createSystem();
```
Creates the "OpenConsole" system with games from database.
Returns `nullptr` if database is empty or initialization failed.

**refreshGameList()**
```cpp
static void refreshGameList();
```
Refreshes the game list from database. Call after scanning or database changes.

### Data Conversion

`OpenConsoleSystem` converts between database and UI representations:

```cpp
GameMetadata (Database) → FileData (UI)
├── name          → getName()
├── description   → getMetadata("desc")
├── coverArtPath  → getMetadata("image")
├── executablePath → getPath()
├── lastPlayed    → getMetadata("lastplayed")
└── playCount     → getMetadata("playcount")
```

### Implementation Details

- **Class**: `OpenConsoleSystem`
- **Pattern**: Singleton (static methods)
- **Location**: `es-app/src/OpenConsoleSystem.h/cpp`
- **Dependencies**: `DatabaseManager`, `PluginManager`, `SystemData`, `FileData`

### Internal Functions

**populateGameListFromDatabase()**
```cpp
static void populateGameListFromDatabase(SystemData* system);
```
Private method that loads all games from database and creates `FileData` objects.

**createFileDataFromGame()**
```cpp
static FileData* createFileDataFromGame(SystemData* system, const GameMetadata& game);
```
Private method that converts a single `GameMetadata` to `FileData`.
Stores database ID in metadata for easy lookups.

### Error Handling

- Missing game files are logged but don't stop processing
- Database initialization failures return `false` from `init()`
- Empty database causes `createSystem()` to return `nullptr`
- All errors logged via `Log::write()`

## Integration Examples

### Example 1: Adding a Custom Settings Menu

```cpp
#include "guis/GuiSettings.h"
#include "guis/GuiVirtualKeyboard.h"

class GuiCustomSettings : public GuiSettings {
public:
    GuiCustomSettings(Window* window) : GuiSettings(window, "CUSTOM SETTINGS") {
        // Add a text entry using virtual keyboard
        ComponentListRow row;
        auto text = std::make_shared<TextComponent>(mWindow, "Custom Path", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
        row.addElement(text, true);
        row.makeAcceptInputHandler([this] {
            openPathEntry();
        });
        addRow(row);
    }

private:
    void openPathEntry() {
        auto keyboard = std::make_shared<GuiVirtualKeyboard>(
            mWindow,
            "ENTER PATH",
            "",
            [this](const std::string& path) {
                // Save the path
                Settings::getInstance()->setString("CustomPath", path);
            }
        );
        mWindow->pushGui(keyboard);
    }
};
```

### Example 2: Triggering a Game Scan

```cpp
#include "GameScanner.h"
#include "OpenConsoleSystem.h"
#include "guis/GuiMsgBox.h"

void triggerScan(Window* window) {
    // Create progress message
    auto msg = std::make_shared<GuiMsgBox>(
        window,
        "Scanning for games...",
        "CANCEL", nullptr
    );
    window->pushGui(msg);

    // Perform scan
    auto stats = GameScanner::scanAllSources([msg](int current, int total, const std::string& status) {
        // Update progress
        msg->setText("Scanning: " + std::to_string(current) + "/" + std::to_string(total));
    });

    // Close progress and show results
    window->removeGui(msg.get());

    std::string resultText =
        "Scan Complete!\n" +
        "Found: " + std::to_string(stats.totalGamesFound) + "\n" +
        "Added: " + std::to_string(stats.newGamesAdded) + "\n" +
        "Duration: " + std::to_string(stats.scanDurationSeconds) + "s";

    window->pushGui(std::make_shared<GuiMsgBox>(
        window,
        resultText,
        "OK", nullptr
    ));

    // Refresh game list
    OpenConsoleSystem::refreshGameList();
}
```

### Example 3: Accessing Database Statistics

```cpp
#include "db/DatabaseManager.h"
#include "guis/GuiMsgBox.h"

void showDatabaseStats(Window* window) {
    auto& db = DatabaseManager::getInstance();

    // Get all games
    auto allGames = db.getAllGames();

    // Count by type
    int appImageCount = 0;
    int renpyCount = 0;
    int debCount = 0;

    for (const auto& game : allGames) {
        switch (game.gameType) {
            case GameType::APPIMAGE: appImageCount++; break;
            case GameType::RENPY: renpyCount++; break;
            case GameType::DEB: debCount++; break;
            default: break;
        }
    }

    // Show statistics
    std::string statsText =
        "Database Statistics\n\n" +
        "Total Games: " + std::to_string(allGames.size()) + "\n" +
        "AppImage: " + std::to_string(appImageCount) + "\n" +
        "Ren'Py: " + std::to_string(renpyCount) + "\n" +
        ".deb: " + std::to_string(debCount);

    window->pushGui(std::make_shared<GuiMsgBox>(
        window,
        statsText,
        "OK", nullptr
    ));
}
```

## Best Practices

### UI Component Design

1. **Always use callbacks for async operations**
   ```cpp
   // Good
   scanGames([this](int progress) { updateUI(progress); });

   // Bad - blocks UI thread
   scanGames();
   updateUI();
   ```

2. **Follow EmulationStation naming conventions**
   - GUI classes: `GuiComponentName`
   - Settings menus: `GuiNameSettings`
   - Use camelCase for methods

3. **Respect theme colors**
   ```cpp
   // Use theme colors
   auto color = mTheme->getColor("textColor");

   // Or use standard colors
   0x777777FF  // Gray text
   0xFFFFFFFF  // White text
   0x00FF00FF  // Green highlight
   ```

4. **Provide clear user feedback**
   - Show progress for long operations
   - Display confirmation dialogs for destructive actions
   - Use descriptive error messages

### Controller Navigation

1. **Standard button mappings**
   - A: Confirm/Select
   - B: Back/Cancel
   - X: Secondary action (e.g., space)
   - Y: Tertiary action (e.g., shift)
   - Start: Accept/Submit

2. **Ensure all UI is navigable with D-pad**
   - No mouse-only operations
   - Clear focus indicators
   - Logical navigation order

3. **Handle cancel gracefully**
   ```cpp
   input->cancelFunc = [this] {
       // Clean up and close
       delete this;
   };
   ```

### Database Integration

1. **Always check initialization**
   ```cpp
   if (!DatabaseManager::getInstance().isInitialized()) {
       Log::write(LogError, "Database not initialized");
       return;
   }
   ```

2. **Use transactions for batch operations**
   ```cpp
   db.beginTransaction();
   for (const auto& game : games) {
       db.insertGame(game);
   }
   db.commitTransaction();
   ```

3. **Handle missing files gracefully**
   ```cpp
   if (!boost::filesystem::exists(game.executablePath)) {
       Log::write(LogWarning, "Game file not found: " + game.name);
       // Continue processing other games
   }
   ```

### Performance

1. **Lazy load resources**
   - Don't load all textures at once
   - Use async loading where possible

2. **Limit database queries**
   - Cache results when appropriate
   - Use prepared statements (DatabaseManager handles this)

3. **Avoid blocking the UI thread**
   - Use callbacks for long operations
   - Show progress indicators

## Testing UI Components

### Manual Testing Checklist

- [ ] Controller navigation works without keyboard/mouse
- [ ] All buttons have expected behavior
- [ ] Cancel/back button works from all states
- [ ] Progress feedback is clear and accurate
- [ ] Error messages are helpful
- [ ] Theme colors are respected
- [ ] Focus indicators are visible
- [ ] Text is readable on different themes

### Common Issues

**Virtual Keyboard Not Appearing**
- Ensure `mWindow->pushGui()` is called
- Check that parent component exists
- Verify callback is valid

**Settings Not Saving**
- Use `Settings::getInstance()->saveFile()` after changes
- Check file permissions on config file
- Verify setting keys match between save/load

**Game List Not Updating**
- Call `OpenConsoleSystem::refreshGameList()` after database changes
- Ensure `SystemData` exists before refreshing
- Check database actually contains games

## Related Documentation

- [Architecture Overview](../ARCHITECTURE.md)
- [Database Layer](../architecture/DATABASE.md)
- [Creating Plugins](CREATING_PLUGINS.md)
- [Configuration Guide](CONFIGURATION.md)

## Future Enhancements

Planned improvements to UI components:

- **GuiDownloadManager**: Progress tracking for game downloads
- **GuiGameDetails**: Detailed game information view
- **GuiPluginConfig**: Per-plugin configuration screens
- **Improved Themes**: OpenConsole-specific theme elements
- **Accessibility**: Screen reader support, high contrast modes
