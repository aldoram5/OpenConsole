# Creating a Custom Plugin

This guide walks you through creating a custom game source plugin for OpenConsole.

## Table of Contents

1. [Overview](#overview)
2. [Plugin Basics](#plugin-basics)
3. [Step-by-Step Tutorial](#step-by-step-tutorial)
4. [Example: USB Scanner Plugin](#example-usb-scanner-plugin)
5. [Testing Your Plugin](#testing-your-plugin)
6. [Best Practices](#best-practices)

## Overview

Plugins allow OpenConsole to discover games from different sources. Each plugin:
- Implements the `IGameSourcePlugin` interface
- Handles authentication (if needed)
- Discovers and returns game metadata
- Optionally supports downloading/installing games

## Plugin Basics

### Interface Requirements

Your plugin must implement:

```cpp
class IGameSourcePlugin {
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

    // Installation (optional)
    virtual bool downloadGame(...) = 0;

    // Capabilities
    virtual bool canHandleGameType(GameType type) const = 0;
    virtual std::vector<GameType> getSupportedGameTypes() const = 0;

    // Lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual std::string getLastError() const = 0;
};
```

## Step-by-Step Tutorial

### Step 1: Create Plugin Files

Create header and implementation files:

```bash
touch es-app/src/plugins/MyPlugin.h
touch es-app/src/plugins/MyPlugin.cpp
```

### Step 2: Define Plugin Class

**MyPlugin.h**:

```cpp
#pragma once
#ifndef ES_APP_PLUGINS_MY_PLUGIN_H
#define ES_APP_PLUGINS_MY_PLUGIN_H

#include "plugins/IGameSourcePlugin.h"
#include <vector>

namespace OpenConsole
{
    class MyPlugin : public IGameSourcePlugin
    {
    public:
        MyPlugin();
        virtual ~MyPlugin();

        // Identification
        std::string getName() const override { return "My Plugin"; }
        std::string getId() const override { return "my_plugin"; }
        std::string getVersion() const override { return "1.0.0"; }

        // Authentication
        bool requiresAuthentication() const override;
        bool isAuthenticated() override;
        AuthResult authenticate() override;
        void logout() override;

        // Game Discovery
        std::vector<GameMetadata> fetchGames() override;

        // Installation
        bool downloadGame(const GameMetadata& game,
            const std::string& installPath,
            DownloadProgressCallback progressCallback) override;

        // Capabilities
        bool canHandleGameType(GameType type) const override;
        std::vector<GameType> getSupportedGameTypes() const override;

        // Lifecycle
        bool initialize() override;
        void shutdown() override;
        std::string getLastError() const override { return mLastError; }

    private:
        bool mInitialized;
        bool mAuthenticated;
        std::string mLastError;
    };

} // namespace OpenConsole

#endif
```

### Step 3: Implement Plugin Logic

**MyPlugin.cpp**:

```cpp
#include "plugins/MyPlugin.h"
#include "Log.h"

namespace OpenConsole
{

MyPlugin::MyPlugin()
    : mInitialized(false)
    , mAuthenticated(false)
{
}

MyPlugin::~MyPlugin()
{
    shutdown();
}

bool MyPlugin::initialize()
{
    if (mInitialized) {
        LOG(LogWarning) << "MyPlugin already initialized";
        return true;
    }

    LOG(LogInfo) << "Initializing MyPlugin...";

    // Perform initialization
    // - Load configuration
    // - Set up connections
    // - Validate environment

    mInitialized = true;
    LOG(LogInfo) << "MyPlugin initialized successfully";
    return true;
}

void MyPlugin::shutdown()
{
    if (!mInitialized)
        return;

    LOG(LogInfo) << "Shutting down MyPlugin";

    // Cleanup
    // - Close connections
    // - Save state
    // - Free resources

    mAuthenticated = false;
    mInitialized = false;
}

bool MyPlugin::requiresAuthentication() const
{
    // Return true if your plugin needs login/API key
    return false;
}

bool MyPlugin::isAuthenticated()
{
    return mAuthenticated;
}

AuthResult MyPlugin::authenticate()
{
    AuthResult result;

    if (requiresAuthentication()) {
        // Implement authentication logic
        // - Display login UI
        // - Verify credentials
        // - Store token

        mAuthenticated = true;
        result.success = true;
        result.userId = "user123";
        result.userName = "User Name";
    } else {
        // No authentication needed
        result.success = true;
        mAuthenticated = true;
    }

    return result;
}

void MyPlugin::logout()
{
    LOG(LogInfo) << "Logging out from MyPlugin";
    mAuthenticated = false;
    // Clear stored credentials
}

std::vector<GameMetadata> MyPlugin::fetchGames()
{
    std::vector<GameMetadata> games;

    if (!mInitialized) {
        LOG(LogError) << "MyPlugin not initialized";
        mLastError = "Plugin not initialized";
        return games;
    }

    LOG(LogInfo) << "Fetching games from MyPlugin...";

    // Implement game discovery logic
    // - Query API
    // - Scan directories
    // - Parse configuration files

    // Example: Add a game
    GameMetadata game;
    game.name = "Example Game";
    game.description = "A great game";
    game.executablePath = "/path/to/game";
    game.gameType = GameType::APPIMAGE;
    game.source = GameSource::LOCAL;
    game.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    games.push_back(game);

    LOG(LogInfo) << "Found " << games.size() << " games";
    return games;
}

bool MyPlugin::downloadGame(const GameMetadata& game,
    const std::string& installPath,
    DownloadProgressCallback progressCallback)
{
    LOG(LogInfo) << "Downloading game: " << game.name;

    // Implement download logic
    // - Fetch game files
    // - Extract archives
    // - Set permissions
    // - Report progress via callback

    if (progressCallback) {
        // Simulate progress
        for (size_t i = 0; i <= 100; i += 10) {
            progressCallback(i, 100);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    return true;
}

bool MyPlugin::canHandleGameType(GameType type) const
{
    // Specify which game types your plugin supports
    return type == GameType::APPIMAGE ||
           type == GameType::DIRECTORY;
}

std::vector<GameType> MyPlugin::getSupportedGameTypes() const
{
    return {
        GameType::APPIMAGE,
        GameType::DIRECTORY
    };
}

} // namespace OpenConsole
```

### Step 4: Register Plugin

Add to `PluginManager::registerBuiltInPlugins()`:

```cpp
void PluginManager::registerBuiltInPlugins()
{
    // Existing plugins...

    // Add your plugin
    auto myPlugin = std::make_shared<MyPlugin>();
    if (registerPlugin(myPlugin)) {
        LOG(LogInfo) << "Registered MyPlugin";
    }
}
```

### Step 5: Update CMakeLists.txt

Add to `es-app/CMakeLists.txt`:

```cmake
set(ES_HEADERS
    # ... existing headers ...
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/MyPlugin.h
)

set(ES_SOURCES
    # ... existing sources ...
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/MyPlugin.cpp
)
```

### Step 6: Rebuild

```bash
cd build
cmake ..
make -j$(nproc)
```

## Example: USB Scanner Plugin

Here's a complete example for scanning USB drives:

```cpp
// USBPlugin.h
#pragma once
#ifndef ES_APP_PLUGINS_USB_PLUGIN_H
#define ES_APP_PLUGINS_USB_PLUGIN_H

#include "plugins/IGameSourcePlugin.h"
#include <set>

namespace OpenConsole
{
    class USBPlugin : public IGameSourcePlugin
    {
    public:
        USBPlugin();
        virtual ~USBPlugin();

        std::string getName() const override { return "USB Scanner"; }
        std::string getId() const override { return "usb"; }
        std::string getVersion() const override { return "1.0.0"; }

        bool requiresAuthentication() const override { return false; }
        bool isAuthenticated() override { return true; }
        AuthResult authenticate() override;
        void logout() override {}

        std::vector<GameMetadata> fetchGames() override;

        bool downloadGame(const GameMetadata& game,
            const std::string& installPath,
            DownloadProgressCallback progressCallback) override;

        bool canHandleGameType(GameType type) const override;
        std::vector<GameType> getSupportedGameTypes() const override;

        bool initialize() override;
        void shutdown() override;
        std::string getLastError() const override { return mLastError; }

    private:
        std::vector<std::string> detectUSBMounts();
        std::vector<GameMetadata> scanUSBDevice(const std::string& mountPoint);

        bool mInitialized;
        std::string mLastError;
    };
}

#endif
```

```cpp
// USBPlugin.cpp
#include "plugins/USBPlugin.h"
#include "launchers/GameLauncherFactory.h"
#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <fstream>

namespace OpenConsole
{

USBPlugin::USBPlugin()
    : mInitialized(false)
{
}

USBPlugin::~USBPlugin()
{
    shutdown();
}

bool USBPlugin::initialize()
{
    if (mInitialized)
        return true;

    LOG(LogInfo) << "Initializing USB Scanner Plugin";
    mInitialized = true;
    return true;
}

void USBPlugin::shutdown()
{
    mInitialized = false;
}

AuthResult USBPlugin::authenticate()
{
    AuthResult result;
    result.success = true;
    return result;
}

std::vector<std::string> USBPlugin::detectUSBMounts()
{
    std::vector<std::string> mounts;

    // Read /proc/mounts to find USB devices
    std::ifstream mtab("/proc/mounts");
    std::string line;

    while (std::getline(mtab, line)) {
        // Look for /media/ or /mnt/ mount points
        if (line.find("/media/") != std::string::npos ||
            line.find("/mnt/usb") != std::string::npos) {

            // Extract mount point
            size_t start = line.find('/');
            size_t end = line.find(' ', start);

            if (start != std::string::npos && end != std::string::npos) {
                std::string mountPoint = line.substr(start, end - start);
                mounts.push_back(mountPoint);
                LOG(LogInfo) << "Found USB mount: " << mountPoint;
            }
        }
    }

    return mounts;
}

std::vector<GameMetadata> USBPlugin::scanUSBDevice(const std::string& mountPoint)
{
    std::vector<GameMetadata> games;

    // Look for Games directory on USB
    std::string gamesDir = Utils::FileSystem::combine(mountPoint, "Games");

    if (!Utils::FileSystem::exists(gamesDir)) {
        LOG(LogDebug) << "No Games directory on USB: " << mountPoint;
        return games;
    }

    LOG(LogInfo) << "Scanning USB games directory: " << gamesDir;

    // Get all files/directories
    std::vector<std::string> items = Utils::FileSystem::getDirContent(gamesDir);

    for (const auto& item : items) {
        std::string fullPath = Utils::FileSystem::combine(gamesDir, item);

        // Detect game type
        GameType type = GameLauncherFactory::detectGameType(fullPath);

        if (type != GameType::UNKNOWN) {
            GameMetadata game;
            game.name = Utils::FileSystem::getStem(fullPath);
            game.executablePath = fullPath;
            game.gameType = type;
            game.source = GameSource::USB;
            game.installPath = gamesDir;

            games.push_back(game);
            LOG(LogInfo) << "Found USB game: " << game.name;
        }
    }

    return games;
}

std::vector<GameMetadata> USBPlugin::fetchGames()
{
    std::vector<GameMetadata> allGames;

    if (!mInitialized) {
        LOG(LogError) << "USBPlugin not initialized";
        return allGames;
    }

    LOG(LogInfo) << "Scanning for USB devices...";

    // Detect USB mounts
    std::vector<std::string> usbMounts = detectUSBMounts();

    if (usbMounts.empty()) {
        LOG(LogInfo) << "No USB devices detected";
        return allGames;
    }

    // Scan each USB device
    for (const auto& mount : usbMounts) {
        std::vector<GameMetadata> games = scanUSBDevice(mount);
        allGames.insert(allGames.end(), games.begin(), games.end());
    }

    LOG(LogInfo) << "Found " << allGames.size() << " games on USB devices";
    return allGames;
}

bool USBPlugin::downloadGame(const GameMetadata& game,
    const std::string& installPath,
    DownloadProgressCallback progressCallback)
{
    // USB games are already accessible, no download needed
    mLastError = "USB games don't need downloading";
    return false;
}

bool USBPlugin::canHandleGameType(GameType type) const
{
    return type == GameType::APPIMAGE ||
           type == GameType::RENPY ||
           type == GameType::DIRECTORY;
}

std::vector<GameType> USBPlugin::getSupportedGameTypes() const
{
    return {
        GameType::APPIMAGE,
        GameType::RENPY,
        GameType::DIRECTORY
    };
}

} // namespace OpenConsole
```

## Testing Your Plugin

### Unit Test Example

```cpp
#include "plugins/MyPlugin.h"
#include <cassert>

void testMyPlugin() {
    MyPlugin plugin;

    // Test initialization
    assert(plugin.initialize() == true);

    // Test identification
    assert(plugin.getId() == "my_plugin");
    assert(plugin.getName() == "My Plugin");

    // Test game fetching
    auto games = plugin.fetchGames();
    assert(games.size() > 0);
    assert(!games[0].name.empty());

    // Test capabilities
    assert(plugin.canHandleGameType(GameType::APPIMAGE));

    // Test shutdown
    plugin.shutdown();

    std::cout << "All tests passed!" << std::endl;
}
```

### Integration Test

```cpp
void testPluginIntegration() {
    // Initialize manager
    PluginManager& pm = PluginManager::getInstance();
    pm.initialize();

    // Get plugin
    auto plugin = pm.getPlugin("my_plugin");
    assert(plugin != nullptr);

    // Fetch games
    auto games = plugin->fetchGames();

    // Add to database
    DatabaseManager& db = DatabaseManager::getInstance();
    db.init("test.db");

    for (auto& game : games) {
        int id = db.insertGame(game);
        assert(id > 0);
    }

    std::cout << "Integration test passed!" << std::endl;
}
```

## Best Practices

### 1. Error Handling

```cpp
std::vector<GameMetadata> fetchGames() override {
    try {
        return performScan();
    } catch (const std::exception& e) {
        LOG(LogError) << "Error fetching games: " << e.what();
        mLastError = e.what();
        return {};
    }
}
```

### 2. Logging

```cpp
LOG(LogInfo) << "Plugin operation started";
LOG(LogDebug) << "Debug detail: " << detail;
LOG(LogWarning) << "Unexpected condition";
LOG(LogError) << "Operation failed: " << error;
```

### 3. Configuration

```cpp
class MyPlugin : public IGameSourcePlugin {
private:
    void loadConfig() {
        // Load from ~/.openconsole/plugins/my_plugin.cfg
        std::string configPath = Utils::FileSystem::getHomePath() +
            "/.openconsole/plugins/my_plugin.cfg";

        // Parse and apply settings
    }
};
```

### 4. Progress Reporting

```cpp
bool downloadGame(..., DownloadProgressCallback callback) override {
    size_t totalSize = getFileSize(url);
    size_t downloaded = 0;

    while (downloading) {
        // Download chunk
        downloaded += chunkSize;

        // Report progress
        if (callback) {
            callback(downloaded, totalSize);
        }
    }
}
```

### 5. Resource Cleanup

```cpp
~MyPlugin() {
    shutdown();  // Ensure cleanup
}

void shutdown() override {
    if (mConnection) {
        mConnection->close();
        mConnection = nullptr;
    }

    if (mTempFiles.size() > 0) {
        for (auto& file : mTempFiles) {
            Utils::FileSystem::removeFile(file);
        }
        mTempFiles.clear();
    }
}
```

## Common Patterns

### Caching

```cpp
std::vector<GameMetadata> fetchGames() override {
    if (isCacheValid()) {
        LOG(LogDebug) << "Returning cached games";
        return mCachedGames;
    }

    mCachedGames = performActualScan();
    mLastCacheTime = std::chrono::system_clock::now();
    return mCachedGames;
}
```

### Batch Processing

```cpp
for (size_t i = 0; i < items.size(); i += BATCH_SIZE) {
    size_t end = std::min(i + BATCH_SIZE, items.size());

    processBatch(items.begin() + i, items.begin() + end);

    // Allow UI updates
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

### Retry Logic

```cpp
bool downloadWithRetry(const std::string& url, int maxRetries = 3) {
    for (int attempt = 0; attempt < maxRetries; attempt++) {
        try {
            return download(url);
        } catch (const NetworkException& e) {
            LOG(LogWarning) << "Download attempt " << (attempt + 1)
                           << " failed: " << e.what();

            if (attempt < maxRetries - 1) {
                std::this_thread::sleep_for(
                    std::chrono::seconds(1 << attempt)  // Exponential backoff
                );
            }
        }
    }

    return false;
}
```

## Related Documentation

- [Plugin System Architecture](../architecture/PLUGINS.md)
- [Plugin API Reference](../api/PLUGIN_API.md)
- [LocalFilesystemPlugin Example](LOCAL_PLUGIN.md)
