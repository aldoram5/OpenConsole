# Plugin System

The OpenConsole plugin system provides an extensible architecture for discovering games from multiple sources. It uses an interface-based design allowing easy addition of new game sources without modifying core code.

## Overview

- **Pattern**: Strategy Pattern + Plugin Architecture
- **Location**: `es-app/src/plugins/`
- **Manager**: `PluginManager` (Singleton)
- **Interface**: `IGameSourcePlugin`
- **Built-in Plugins**: LocalFilesystemPlugin, ItchIoPlugin (planned)

## Architecture

```
┌─────────────────────────────────────┐
│       PluginManager                 │
│        (Singleton)                  │
├─────────────────────────────────────┤
│  - registerPlugin()                 │
│  - getPlugin()                      │
│  - getAllPlugins()                  │
│  - refreshAllGames()                │
└─────────────────────────────────────┘
            │
            │ manages
            ↓
┌───────────────────────────────┐
│    IGameSourcePlugin          │
│      (interface)              │
├───────────────────────────────┤
│  + getName()                  │
│  + authenticate()             │
│  + fetchGames()               │
│  + downloadGame()             │
│  + getSupportedGameTypes()    │
└───────────────────────────────┘
            △
            │ implements
    ┌───────┴────────┐
    │                │
┌───────────────┐  ┌──────────────┐
│ LocalFilesystemPlugin │  │ ItchIoPlugin │
│   Plugin        │  │   (planned)  │
└───────────────┘  └──────────────┘
```

## Plugin Interface

### IGameSourcePlugin

All plugins must implement this interface:

```cpp
class IGameSourcePlugin {
public:
    // Identification
    virtual std::string getName() const = 0;
    virtual std::string getId() const = 0;
    virtual std::string getVersion() const = 0;

    // Authentication
    virtual bool requiresAuthentication() const = 0;
    virtual bool isAuthenticated() = 0;
    virtual AuthResult authenticate() = 0;
    virtual void logout() = 0;

    // Game Discovery
    virtual std::vector<GameMetadata> fetchGames() = 0;

    // Game Installation
    virtual bool downloadGame(const GameMetadata& game,
        const std::string& installPath,
        DownloadProgressCallback progressCallback = nullptr) = 0;

    // Capabilities
    virtual bool canHandleGameType(GameType type) const = 0;
    virtual std::vector<GameType> getSupportedGameTypes() const = 0;

    // Lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Error Handling
    virtual std::string getLastError() const = 0;
};
```

## Plugin Manager

### Initialization

```cpp
PluginManager& pm = PluginManager::getInstance();
pm.initialize();  // Auto-registers built-in plugins
```

### Plugin Registration

```cpp
// Create plugin instance
auto myPlugin = std::make_shared<MyCustomPlugin>();

// Register with manager
pm.registerPlugin(myPlugin);

// Initialize the plugin
myPlugin->initialize();
```

### Querying Plugins

```cpp
// Get specific plugin by ID
auto plugin = pm.getPlugin("local");

// Get all plugins
auto allPlugins = pm.getAllPlugins();

// Get authenticated plugins only
auto authPlugins = pm.getAuthenticatedPlugins();

// Get plugins supporting specific game type
auto appImagePlugins = pm.getPluginsByGameType(GameType::APPIMAGE);

// Check if plugin exists
if (pm.hasPlugin("itch_io")) {
    // ...
}
```

### Fetching Games

```cpp
// Fetch from all authenticated plugins
std::vector<GameMetadata> allGames = pm.refreshAllGames();

// Fetch from specific plugin
std::vector<GameMetadata> localGames = pm.refreshGamesFromPlugin("local");
```

## Built-In Plugins

### LocalFilesystemPlugin

Scans local directories for games.

**ID**: `"local"`

**Features**:
- Recursive directory scanning
- Configurable scan paths
- Auto game type detection
- Cover art discovery
- Name sanitization

**Configuration**:
```cpp
auto plugin = std::dynamic_pointer_cast<LocalFilesystemPlugin>(
    pm.getPlugin("local"));

// Add custom scan path
plugin->addScanPath("/mnt/games");

// Configure scanning
plugin->setRecursiveScan(true);
plugin->setMaxScanDepth(5);

// Scan specific directory
auto games = plugin->scanDirectory("/path/to/games");
```

**Default Scan Paths**:
- `~/Games`
- `~/.local/share/games`
- `/usr/local/games`
- `/media/usb0/Games`
- `/media/games`

**See**: [LocalFilesystemPlugin Details](../guides/LOCAL_PLUGIN.md)

### ItchIoPlugin (Planned)

Integrates with itch.io API.

**ID**: `"itch_io"`

**Features**:
- OAuth authentication
- Browse owned games
- Download purchased games
- Version checking
- Update notifications

**Authentication Flow**:
1. Display authorization URL
2. User authorizes on itch.io
3. User enters API key
4. Plugin stores encrypted token
5. Plugin validates authentication

**See**: [ItchIoPlugin Specification](../guides/ITCH_PLUGIN.md)

## Creating a Custom Plugin

### Step 1: Implement Interface

```cpp
#include "plugins/IGameSourcePlugin.h"

class MyCustomPlugin : public IGameSourcePlugin {
public:
    std::string getName() const override {
        return "My Custom Source";
    }

    std::string getId() const override {
        return "my_custom";
    }

    std::string getVersion() const override {
        return "1.0.0";
    }

    bool requiresAuthentication() const override {
        return false;
    }

    bool isAuthenticated() override {
        return true;
    }

    AuthResult authenticate() override {
        AuthResult result;
        result.success = true;
        return result;
    }

    void logout() override {}

    std::vector<GameMetadata> fetchGames() override {
        std::vector<GameMetadata> games;

        // Your discovery logic here
        GameMetadata game;
        game.name = "Custom Game";
        game.executablePath = "/path/to/game";
        game.gameType = GameType::APPIMAGE;
        game.source = GameSource::LOCAL;

        games.push_back(game);
        return games;
    }

    bool downloadGame(const GameMetadata& game,
        const std::string& installPath,
        DownloadProgressCallback progressCallback) override {
        // Download logic
        return false;
    }

    bool canHandleGameType(GameType type) const override {
        return type == GameType::APPIMAGE;
    }

    std::vector<GameType> getSupportedGameTypes() const override {
        return { GameType::APPIMAGE };
    }

    bool initialize() override {
        LOG(LogInfo) << "MyCustomPlugin initialized";
        return true;
    }

    void shutdown() override {
        LOG(LogInfo) << "MyCustomPlugin shutdown";
    }

    std::string getLastError() const override {
        return mLastError;
    }

private:
    std::string mLastError;
};
```

### Step 2: Register Plugin

```cpp
// In PluginManager::registerBuiltInPlugins() or at runtime:
auto customPlugin = std::make_shared<MyCustomPlugin>();
PluginManager::getInstance().registerPlugin(customPlugin);
```

### Step 3: Use Plugin

```cpp
auto games = PluginManager::getInstance()
    .refreshGamesFromPlugin("my_custom");
```

## Authentication

### AuthResult Structure

```cpp
struct AuthResult {
    bool success;                  // Authentication succeeded
    std::string errorMessage;      // Error details (if failed)
    std::string userId;            // Optional user ID
    std::string userName;          // Optional display name
};
```

### Authentication Flow

```cpp
auto plugin = pm.getPlugin("itch_io");

if (plugin->requiresAuthentication() && !plugin->isAuthenticated()) {
    AuthResult result = plugin->authenticate();

    if (result.success) {
        LOG(LogInfo) << "Authenticated as: " << result.userName;
    } else {
        LOG(LogError) << "Authentication failed: " << result.errorMessage;
    }
}
```

## Download Management

### Progress Callbacks

```cpp
using DownloadProgressCallback = std::function<void(size_t bytesDownloaded, size_t totalBytes)>;

// Example callback
auto progressCallback = [](size_t downloaded, size_t total) {
    float percent = (float)downloaded / total * 100.0f;
    std::cout << "Progress: " << percent << "%" << std::endl;
};

// Download game
bool success = plugin->downloadGame(
    gameMetadata,
    "/home/user/Games",
    progressCallback
);
```

## Plugin Lifecycle

### Initialization Order

1. PluginManager created (singleton)
2. `PluginManager::initialize()` called
3. Built-in plugins registered
4. Each plugin's `initialize()` called
5. Plugins ready for use

### Shutdown Order

1. `PluginManager::shutdown()` called
2. Each plugin's `shutdown()` called
3. Plugins unregistered
4. PluginManager destroyed

### Manual Management

```cpp
// Unregister plugin
pm.unregisterPlugin("my_custom");

// Re-register
auto newInstance = std::make_shared<MyCustomPlugin>();
pm.registerPlugin(newInstance);
```

## Error Handling

### Plugin-Level Errors

```cpp
auto games = plugin->fetchGames();
if (games.empty()) {
    std::string error = plugin->getLastError();
    LOG(LogError) << "Plugin error: " << error;
}
```

### Manager-Level Errors

```cpp
try {
    auto games = pm.refreshAllGames();
} catch (const std::exception& e) {
    LOG(LogError) << "Plugin exception: " << e.what();
}
```

## Best Practices

### 1. Handle Initialization Failures

```cpp
bool initialize() override {
    if (!performSetup()) {
        mLastError = "Setup failed";
        return false;
    }
    return true;
}
```

### 2. Implement Proper Cleanup

```cpp
void shutdown() override {
    // Close connections
    // Release resources
    // Save state
}
```

### 3. Validate Game Data

```cpp
std::vector<GameMetadata> fetchGames() override {
    std::vector<GameMetadata> games;

    for (auto& game : discoveredGames) {
        // Validate before adding
        if (game.name.empty() || game.executablePath.empty()) {
            LOG(LogWarning) << "Invalid game data, skipping";
            continue;
        }
        games.push_back(game);
    }

    return games;
}
```

### 4. Provide Detailed Errors

```cpp
mLastError = "Failed to connect to API: " + errorDetails;
```

### 5. Log Important Events

```cpp
LOG(LogInfo) << "Scanning started: " << scanPath;
LOG(LogDebug) << "Found game: " << game.name;
LOG(LogWarning) << "Authentication expired";
LOG(LogError) << "Download failed: " << error;
```

## Performance Considerations

### Async Operations

For long-running operations, consider async patterns:

```cpp
std::vector<GameMetadata> fetchGames() override {
    // Quick return of cached data
    if (mCachedGames.size() > 0 && !needsRefresh()) {
        return mCachedGames;
    }

    // Perform actual scan
    mCachedGames = performScan();
    return mCachedGames;
}
```

### Caching

Cache expensive operations:

```cpp
private:
    std::vector<GameMetadata> mCachedGames;
    std::chrono::system_clock::time_point mLastScan;

    bool needsRefresh() {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
            now - mLastScan).count();
        return elapsed > 30;  // Refresh every 30 minutes
    }
```

### Batch Operations

Process in batches to provide feedback:

```cpp
std::vector<GameMetadata> fetchGames() override {
    std::vector<GameMetadata> games;

    for (const auto& dir : scanPaths) {
        auto dirGames = scanSingleDirectory(dir);
        games.insert(games.end(), dirGames.begin(), dirGames.end());

        // Allow UI updates between batches
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return games;
}
```

## Security Considerations

### Credential Storage

Never store credentials in plain text:

```cpp
// Use encryption for sensitive data
std::string encryptedToken = Crypto::encrypt(apiToken, masterKey);
saveToFile(encryptedToken);
```

### API Key Protection

```cpp
// Don't log sensitive data
LOG(LogDebug) << "API call successful";  // Good
// LOG(LogDebug) << "API key: " << apiKey;  // BAD!
```

### Path Validation

```cpp
bool isValidPath(const std::string& path) {
    // Prevent path traversal
    if (path.find("..") != std::string::npos) {
        return false;
    }

    // Check if path exists and is accessible
    return Utils::FileSystem::exists(path) &&
           Utils::FileSystem::isReadable(path);
}
```

## Testing

### Unit Testing

```cpp
TEST(MyCustomPlugin, FetchGames) {
    MyCustomPlugin plugin;
    plugin.initialize();

    auto games = plugin.fetchGames();

    EXPECT_GT(games.size(), 0);
    EXPECT_FALSE(games[0].name.empty());
}
```

### Mock Plugins

```cpp
class MockPlugin : public IGameSourcePlugin {
    // Implement with test data
};
```

## Related Documentation

- [Plugin Creation Guide](../guides/CREATING_PLUGINS.md)
- [LocalFilesystemPlugin Details](../guides/LOCAL_PLUGIN.md)
- [Plugin API Reference](../api/PLUGIN_API.md)
- [Architecture Overview](../ARCHITECTURE.md)
